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
*   @file   mstypes.h
*
*   @brief  This file contains the common defines used by all layers
*
****************************************************************************/

#ifndef _MSTYPES_H_
#define _MSTYPES_H_

#ifndef _CAPI2_MSTYPES_H_
#include "ms.h"
#include "timer.h"

#include "mobcom_types.h"

#include "consts.h"
#include "mti_build.h"
#include "resultcode.h"
#endif

//******************************************************************************
//
// Global Types 
//
//******************************************************************************
#define GPRS_SVC_CODE			99		///<	GPRS service code
#define GPRS_CTXTID_MIN			1
#define GPRS_CTXTID_MAX			10

//******************************************************************************
//
// Alphabet Character Set
//
//******************************************************************************

#define	CODE_PAGE_MTI_AT_SIGN	0x80		///< Used to mark '@' sign for CodePageMTI
#define	DEF_GSM_AT_SIGN			0x00		///< Used to mark '@' sign for Default GSM

typedef UInt8		CodePageMTI_t;			///< Code Page MTI Character Set

/**
Enum:	Database Element Type
		This enum is used for defining the type of elements in the database.
IMPORTANT NOTE: Please add any new element particular to a functional area in that section.

@internal
While adding a new element the checklist below should be followed:
1.	Update MS_Element_t enum in mstypes.h to add the new element name
    as per the naming conventions and in the correct section ( e.g ss element should be grouped together etc)
2.	Add code to the correct local function for setting and retrieving the element value ( MS_SetLocalSSElem, MS_GetLocalPhCtrlElem etc)
3.	Update the MS_GetElementFuncTable[] and MS_SetElementFuncTable[] to provide the appropriate function pointer corresponding to the elementtype.
    ** It is mandatory to provide the funcptr for both the Get and Set tables.
4.	Update the ms_database document (Sec 2.4) to add the element name and type and any comments and check it back in.
	( http://sp%2Dmpg%2Ebroadcom%2Ecom/mobcom/SW-Platform_Framework/Shared%20Documents/CAPI/Design/MS_DatabaseDesign.doc )
 

**/


typedef enum
{
	MS_ELEM_INVALID,								///< Invalid  

	//----SS Local Elements
	MS_LOCAL_SS_ELEM_CLIP, 							///< CLIP  
	MS_LOCAL_SS_ELEM_CLIR,							///< CLIR  
	MS_LOCAL_SS_ELEM_COLP,							///< COLP  
	MS_LOCAL_SS_ELEM_CCWA,							///< CCWA  
	MS_LOCAL_SS_ELEM_FDN_CHECK,						///< SS FDN check
	MS_LOCAL_SS_ELEM_NOTIFICATION_SWITCH,			///< SS Notification Switch
	MS_LOCAL_SS_ELEM_TX_USSD_DATA_ENCODE,			///< Whether the Tx USSD phase 2 data is encoded by the client as per DCS (1/0 : encoded/not encoded)
	MS_LOCAL_SS_ELEM_RX_USSD_DATA_ENCODE,			///< Whether the Rx USSD phase 2 data shall be encoded by the MNSS module as per DCS (1/0 : encoded/not encoded)
	MS_LOCAL_SS_ELEM_INCLUDE_FACILITY_IE,			///<Element type {::UInt8}, Include Facility IE in the res/ind/data for the client
	MS_LOCAL_SS_ELEM_INVOKE_ID,						///<Element type {::UInt8}, Invoke Identifier for each MO SS related session


	//----Test Local Elements
	MS_LOCAL_TEST_ELEM_CHAN,						///< the test frequencies for each band  
	MS_LOCAL_TEST_ELEM_PROD_DATA,					///< the production test data stored for all Bands  
	MS_LOCAL_TEST_ELEM_PROD_MODE,					///< if 0 - disable production test mode else  
													///< enables production test mode.  
	//----CC Local Elements
	MS_LOCAL_CC_ELEM_AUDIO,							///< AUDIO control in MNCC enable/disable 
	MS_LOCAL_CC_ELEM_VIDEO_CALL,					///< Video call feature  
	MS_LOCAL_CC_ELEM_DEF_VOICE_BC,					///< Default Bearer Capability used for voice call initiation  
	MS_LOCAL_CC_ELEM_CLIENT_GEN_TONE,				///< whether client generated tone is enabled 
	MS_LOCAL_CC_ELEM_SCUDIF,						///< whether the SCUDIF feature is enabled of disabled
	MS_LOCAL_CC_ELEM_SPEECH_CODEC_LIST,				///< speech codec list to be used in call control messages, this should change if the channel mode supported changes

	MS_LOCAL_CC_ELEM_CURR_CALL_CFG,					///< Current call configuration (type CallCfg_t)
	MS_LOCAL_CC_ELEM_HSCSD_REPORT,					///< reported in ATDS_REPORT_IND message
	MS_LOCAL_CC_ELEM_IS_HSCSD_CALL,					///< boolean true if CSD/HSCSD call
	MS_LOCAL_CC_ELEM_RX_SLOTS,						///< current call receive slots	
	MS_LOCAL_CC_ELEM_TX_SLOTS,						///< current call transmit slots
	MS_LOCAL_CC_ELEM_AIR_IF_USER_RATE,				///< current call air interface user rate
	MS_LOCAL_CC_ELEM_CURRENT_CODING,				///< current call codings (9600/14400 bps per slot)
	MS_LOCAL_CC_ELEM_CURRENT_CONNECT_ELEMENT,		///< Current call connection element(Transparent vs. non-transparent)
	MS_LOCAL_CC_ELEM_TEL_NUMBER,					///< Telephone number
	MS_LOCAL_CC_ELEM_CALLING_SUBADDR,				///< Sub Address of the calling station
	MS_LOCAL_CC_ELEM_CALLED_SUBADDR,				///< Sub Address of the called station
	MS_LOCAL_CC_ELEM_CONNECT_SUBADDR,				///< Connection sub address
	MS_LOCAL_CC_ELEM_CRLP,							///< Radio Link Protocol parameters	
	MS_LOCAL_CC_ELEM_CBST ,							///< Bearer Service parameters
	MS_LOCAL_CC_ELEM_DS_NEG_DATACOMP,				///< Data compression parameters negotiated between peers
	MS_LOCAL_CC_ELEM_DS_REQ_DATACOMP,				///< Data compression parameters requested by host
	MS_LOCAL_CC_ELEM_DS_REQ_SUCCESS_NEG,			///< Boolean value to indicate if DC negotiation is necessary for call completion
	MS_LOCAL_CC_ELEM_EC_REQUESTED,					///< Error correction parameter requested
	MS_LOCAL_CC_ELEM_EC_FALLBACK_NOT_NEGOTIABLE,	///< Error correction parameter fall back value if negotiation is not possible
	MS_LOCAL_CC_ELEM_EC_FALLBACK_NO_ANSWER,			///< Error correction parameter answer fall back value
	MS_LOCAL_CC_ELEM_EC_PARAMS,						///< Negotiated Error correction parameters
	MS_LOCAL_CC_ELEM_DC_PARAMS,						///< Negotiated Data compression parameters
	MS_LOCAL_CC_ELEM_MCLASS,						///< Multi Slot class of the mobile
	MS_LOCAL_CC_ELEM_MAX_RX_SLOTS,					///< Maximum Receive slots
	MS_LOCAL_CC_ELEM_MAX_TX_SLOTS,					///< Maximmum transmit slots
	MS_LOCAL_CC_ELEM_SUM_SLOTS,						///< Sum of slots
	MS_LOCAL_CC_ELEM_CODINGS,						///< Channel Codings ie 9600/14400 bps slots
	MS_LOCAL_CC_ELEM_WANTED_RX_SLOTS,				///< Number of wanted receive time slots
	MS_LOCAL_CC_ELEM_WANTED_AIR_USER_RATE,			///< Wanted Air Interface User Rate
	MS_LOCAL_CC_ELEM_MAX_TX_SLOTS_SUPPORTED,		///< Maximum number of transmit slots that can be supported
	MS_LOCAL_CC_ELEM_CURR_TI_PD,					///< Current transaction identifier and protocol discriminator
	MS_LOCAL_CC_ELEM_CURR_SERVICE_MODE,				///< Current service mode
	MS_LOCAL_CC_ELEM_DS_DATACOMP_IND,				///< Remote Compression indication 
	MS_LOCAL_CC_ELEM_CUGI,							///< Closed user group index: 0 - 10
	MS_LOCAL_CC_ELEM_SPKR_LOUDNESS,					///< for speaker loudness
	MS_LOCAL_CC_ELEM_SPKR_MONITOR,					///< for speaker monitor
	MS_LOCAL_CC_ELEM_CALL_METER,					///< Current Call Meter
	MS_LOCAL_CC_ELEM_ENABLE_AUTO_REJECT,			///< auto reject MT voice call
	MS_LOCAL_CC_ELEM_ETBM,							///< support of AT+ETBM command.
	MS_LOCAL_CC_ELEM_CLIR,							///< Calling Line ID Restriction
	
	MS_LOCAL_CC_ELEM_FAX_PARAMS,					///< Fax call related paramters (type FaxParam_t)
	MS_LOCAL_CC_ELEM_FAX_FCLASS,					///< Fax class
    MS_LOCAL_CC_ELEM_END_CALL_CAUSE,                ///< End call cause


	//----SMS Local Elements
	MS_LOCAL_SMS_ELEM_DEF_REC_NUMBER,				///< SMS Default Recorde Number
	MS_LOCAL_SMS_ELEM_CLIENT_HANDLE_MT_SMS,			///< Client Handles MT SMS
	MS_LOCAL_SMS_ELEM_MORE_MESSAGE_TO_SEND,			///< More Message to Send
	
	//----SATK Local Elements
	MS_LOCAL_SATK_ELEM_SETUP_CALL_CTR,				///< Whether BRCM STK module handles STK Setup Call  
	MS_LOCAL_SATK_ELEM_SEND_SS_CTR,					///< Whether BRCM STK module handles STK Send SS  
	MS_LOCAL_SATK_ELEM_SEND_USSD_CTR,				///< Whether BRCM STK module handles STK Send USSD  
	MS_LOCAL_SATK_ELEM_SEND_SMS_CTR,		//0x10	///< Whether BRCM STK module handles STK Send SMS   
	MS_LOCAL_SATK_ELEM_SEND_DTMF_CTR,				///< Whether BRCM STK module handles STK Send DTMF  
	MS_LOCAL_SATK_ELEM_PLAY_TONE_CTR,				///< Whether BRCM STK module handles STK Play Tone  
	MS_LOCAL_SATK_ELEM_CALL_CONTROL_CTR,			///< Whether BRCM STK module handles STK voice Call Control by SIM
	MS_LOCAL_SATK_ELEM_SS_CONTROL_CTR,				///< Whether BRCM STK module handles STK SS Call Control by SIM
	MS_LOCAL_SATK_ELEM_USSD_CONTROL_CTR,	//0x15	///< Whether BRCM STK module handles STK USSD Call Control by SIM
	MS_LOCAL_SATK_ELEM_SMS_CONTROL_CTR,				///< Whether BRCM STK module handles STK SMS Control by SIM
	MS_LOCAL_SATK_ELEM_GENERIC_INTERFACE_CTR,		///< Whether BRCM STK module uses generic data interface for most STK commands
	
    //Phonebook Elements
    MS_LOCAL_PBK_ELEM_BDN_CACHE_CTR,                ///< Whether BRCM PBK module supports BDN caching

	MS_LOCAL_PHCTRL_ELEM_EBSE,						///< EBSE the band selected through EBSE  
	MS_LOCAL_PHCTRL_ELEM_PLMN_SESSION,				///< current plmn session - refer PLMNSession_t 
	MS_LOCAL_PHCTRL_ELEM_PLMN_MODE,					///< element should be accessed using PlmnSelectMode_t type  
	MS_LOCAL_PHCTRL_ELEM_PREV_PLMN_MODE,   			///< previous PLMN selection mode; accessed using PlmnSelectMode_t 
	MS_LOCAL_PHCTRL_ELEM_PLMN_SELECT_RAT,			///< GSM,UMTS or both, accessed using UInt8  
	MS_LOCAL_PHCTRL_ELEM_PREV_PLMN_SELECT_RAT,		///< rat for the selected plmn  
	MS_LOCAL_PHCTRL_ELEM_PLMN_FORMAT,				///< refer PlmnSelectFormat_t  
	MS_LOCAL_PHCTRL_ELEM_ABORT_PLMN_SELECT,			///< access using Boolean
	MS_LOCAL_PHCTRL_ELEM_USE_PNN_NAME,              ///< used first EF-PNN record as network name, for T-Mobile 7721 
	MS_LOCAL_PHCTRL_ELEM_PCM_MODE,					///< 1=Digital PCM interface enabled; 0=disabled,UInt8 
	MS_LOCAL_PHCTRL_ELEM_IS_PWRDWN_SOFTRESET, //0x21///< should be accessed using Boolean  
	MS_LOCAL_PHCTRL_ELEM_CGEREP, 					///< group event reporting details  
	MS_LOCAL_PHCTRL_ELEM_SELECTED_PLMN,				///< the current selected plmn  
	MS_LOCAL_PHCTRL_ELEM_PREV_SEL_PLMN, 			///< previous selected plmn  
	MS_LOCAL_PHCTRL_ELEM_PREF_USIM_PBK,				///< preferred USIM phonebook selection
	MS_LOCAL_PHCTRL_ELEM_CSCS,						///< the character set to be used for conversion of char between TE and MT
	MS_LOCAL_PHCTRL_ELEM_CPWC,						///< the power class for all the gsm frequency bands, set by the user
	MS_LOCAL_PHCTRL_ELEM_PWRDWN_DETACH_TIMER,		///< value for the power down detach timer
	MS_LOCAL_PHCTRL_ELEM_PWRDWN_STORAGE_TIMER,		///< value for power down storage timer
	MS_LOCAL_PHCTRL_ELEM_ATTACH_MODE,				///< attach mode, can be GSM, GPRS, GSM_GPRS.This is persistent
	MS_LOCAL_PHCTRL_ELEM_USER_NW_RESELECT,			///< whether user network reselection is triggered	
	MS_LOCAL_PHCTRL_ELEM_WAIT_BEFORE_DETACH_TIME,	///< used to define the time MS should wait before detach after pdp deactivation or 
													///< deactivation failure when the attach mode is GSM_ONLY
	MS_LOCAL_PHCTRL_ELEM_SMSME_STORAGE_ENABLE,		///< value if set will denote the SMS ME storage is enabled	
	MS_LOCAL_PHCTRL_ELEM_SMSME_MAX_CAPACITY,		///< SMS ME storage capacity
    MS_LOCAL_PHCTRL_ELEM_IMEI,		                ///< IMEI
    MS_LOCAL_PHCTRL_ELEM_SW_VERSION,				///< ( Software version )
	MS_LOCAL_PHCTRL_ELEM_FORCE_SIM_NOT_RDY,			///< Force "SIM not ready" status so that we send "NO SIM" status in ATTACH_REQ to stack

	MS_LOCAL_PHCTRL_CLIENT_BAND,					///< Client Band
	MS_LOCAL_PHCTRL_CLIENT_RAT,						///< Client Rat
	MS_LOCAL_PHCTRL_ELEM_CURR_ATTACH_MODE,			///< reflects the current status of attach required by user (changed by cgatt and ATTACH_MODE)
	
	//-----General AT Elements
	MS_LOCAL_AT_ELEM_AUDIO_CTRL,					///< AT audio control setting  

	//-----General MS Elements
	MS_LOCAL_MS_ELEM_ASYNC_RETURN_RESULT,			///< Whether the platform shall send syn/asyncronous response
	MS_LOCAL_MS_ELEM_DIALOG_ID,						///<Element type {::UInt32}, Dialog Identifier for each (i.e. SS, etc.) session

	//------Stack Elements
	
	MS_STACK_ELEM_NVRAM_CLASSMARK,					///< indicates the nvram classmark stored in stack	
	MS_STACK_ELEM_DTX_STATUS,						///< status of dtx, dtx can be enabled/disabled by upper layers
	MS_STACK_ELEM_CELL_BARR_STATUS,					///< used by upper layer to notify stack if it can camp on barred cells or not
	MS_STACK_ELEM_IS_DEDICATED_MODE,				///< used by upper layer to know if UE is in dedicated mode (either CS or PS )												 
	MS_STACK_ELEM_CURR_TIMING_ADVANCE,				///< used by upper layer to know the current timing advance from stack
	MS_STACK_ELEM_SPEECHCODEC_BITMAP,				///< used by upper layer to set the speech codecs supported as set by the user
	MS_STACK_ELEM_CHANNELMODE_SUPPORTED,			///< used for retreiveing the channel mode used by CAPI and stack.

	//----- DRIVER Elements	
	
	MS_DRIVER_ELEM_SPINNER_SLEEP_MODE,				///< SPINNER sleep mode 

	//-----Network Elements

	MS_NETWORK_ELEM_DTM,							///< DTM     
	MS_NETWORK_ELEM_HSDPA_SUPPORT,					///< HSDPA_SUPPORT  
	MS_NETWORK_ELEM_HSUPA_SUPPORT,					///< HSUPA_SUPPORT  
	MS_NETWORK_ELEM_GPRS_SUPPORT,					///< GPRS SUPPORT.
	MS_NETWORK_ELEM_EGPRS_SUPPORT,					///< EGPRS_SUPPORT.
 	MS_NETWORK_ELEM_NOM,							///< Network operation mode of the network the UE is camped on  
	MS_NETWORK_ELEM_NW_TYPE,						///< TYPE of PLMN that the UE is camped on  
	MS_NETWORK_ELEM_RAC,							///< RAC of the routing area that the UE is in.  
	MS_NETWORK_ELEM_RNC,			    		 	///< RNC (Radio Network Controller) the UE is in.  
	MS_NETWORK_ELEM_GSMREG_STATE,					///< The current mobile gsm registration state    
	MS_NETWORK_ELEM_GPRSREG_STATE,  				///< The current mobile gprs registration state  
	MS_NETWORK_ELEM_GSM_CAUSE,						///< The cause of gsm registration state  
	MS_NETWORK_ELEM_GPRS_CAUSE,	 					///< GPRS registration state cause 
	MS_NETWORK_ELEM_NET_INFO,						///< Network information type refer MSNetworkInfo_t  
	MS_NETWORK_ELEM_GSM_NW_CAUSE,			 		///< the GSM registration cause sent by network  
	MS_NETWORK_ELEM_GPRS_NW_CAUSE,					///< the GPRS registratio cause sent by n/w 
	MS_NETWORK_ELEM_REG_INFO,						///< refer MSRegInfo_t used to send back to user  
	MS_NETWORK_ELEM_REGSTATE_INFO,					///< this provides the elements in MSRegStateInfo_t  
	MS_NETWORK_ELEM_ISGSM_REGISTERED,				///< this indicates whether we GSM registered or not  
	MS_NETWORK_ELEM_ISGPRS_REGISTERED,				///< this indicates whether we are registered for GPRS 
	MS_NETWORK_ELEM_CSINVALID_SIM_MS_ME_STATUS,		///< Invalid SIM/MS/ME CS status  
	MS_NETWORK_ELEM_GPRSINVALID_SIM_MS_ME_STATUS,	///< Invalid SIM/MS/ME status for GPRS
	MS_NETWORK_ELEM_IS_UMTS_DCH_STATE,				///< TRUE if UMTS is in Cell-DCH state
	MS_NETWORK_RADIO_STATUS,                        ///< Radio status (i.e. whether UE in TBF state)
	MS_NETWORK_ELEM_CURRENT_CAMPED_PLMN,			///<Element type {::PLMNId_t}, the current camped plmn, this is updated from the cell_info_ind sent by stack
													//								isForbidden element in this context is not valid and is not filled in.
	
    //-----SIM Elements

    MS_SIM_ELEM_PIN1_STATUS,                        ///< SIM PIN1 Status
    MS_SIM_ELEM_PIN2_STATUS,                        ///< SIM PIN2 Status
    MS_SIM_ELEM_ICC_ID,                             ///< SIM ASCII ICC ID
    MS_SIM_ELEM_RAW_ICC_ID,                         ///< SIM RAW ICC ID
    MS_SIM_ELEM_SST,                                ///< SIM Service Table
    MS_SIM_ELEM_CPHS_ONS,                           ///< CPHS Operator Name String
    MS_SIM_ELEM_CPHS_ONS_LEN,                       ///< CPHS ONS Length
    MS_SIM_ELEM_CPHS_ONSS,                          ///< CPHS Short Operator Name String
    MS_SIM_ELEM_CPHS_ONSS_LEN,                      ///< CPHS ONSS Length
    MS_SIM_ELEM_CPHS_INFO,                          ///< CPHS Info
    MS_SIM_ELEM_ECC_REC_LIST,                       ///< MS_SIM_ELEM_ECC_REC_LIST
    
	MS_MAX_ELEMENT_TYPE								///< introduce new element type before value

} MS_Element_t;	///< MS Element Type

//******************************************************************************
//
// GSM Types
//
//******************************************************************************
/**
	Coding scheme for alpha data in SIM, as defined in Annex B. of GSM 11.11 
**/
typedef enum
{
	/* Note: do not change the following enum values!!!!! */
	ALPHA_CODING_GSM_ALPHABET = 0x00,	///< Default 
	ALPHA_CODING_UCS2_80 = 0x80,
	ALPHA_CODING_UCS2_81 = 0x81,
	ALPHA_CODING_UCS2_82 = 0x82
} ALPHA_CODING_t;

/* Represent empty bytes in the SIM */
#define	SIM_RAW_EMPTY_VALUE				0xFF		///< Empty bytes in the SIM
#define	SIM_RAW_EMPTY_VALUE_TWO_BYTES	0xFFFF		///< Empty two bytes in the SIM
#define	SIM_RAW_EMPTY_VALUE_FOUR_BYTES	0xFFFFFFFF	///< Empty four bytes in the SIM

/**
	GSM Type of Number
**/
typedef enum {								
	UnknownTON			= 0,	///< Unknown			
	InternationalTON	= 1,	///< International
	NationalTON			= 2,	///< National
	NetworkSpecificTON	= 3,	///< Network specific type
	DedicatedAccessTON	= 4,	///< Dedicated access
	DefaultAlphabetTON  = 5		///< Default Apha
} gsm_TON_t;///< Type of Number

/**
	GSM Number Plan Identifier
**/

typedef enum {								
	UnknownNP			= 0,		///< Unknown
	ISDNNP				= 1,		///< ISDN Number Plan
	DataNP				= 3,		///< Data Number
	TelexNP				= 4,		///< Telex type Number plan
	NationalNP			= 8,		///< National Number Plan
	PrivateNP			= 9			///< Private type
} gsm_NPI_t;///< Number Plan Identifier

#define	INTERNATIONAL_CODE	'+'				///< '+' at the beginning
											///< of a number represents an international
											///< phone number

#define VOICE_CALL_CODE	';'			///< ';' at the end of a dialing string indicates it is voice call

/* Combined value to identify two important parameters of a phone number:
 * 1. Type of Number
 * 2. Numbering Plan
 * See section 10.5.4.7 of GSM 04.08. 
 */
#define UNKNOWN_TON_UNKNOWN_NPI ((UInt8) (0x80 | (UnknownTON << 4) | UnknownNP))	///< Unknown Type of Number and unknown Numbering Plan 
#define UNKNOWN_TON_ISDN_NPI	((UInt8) (0x80 | (UnknownTON << 4) | ISDNNP))		///< Unknown Type of Number and ISDN Numbering Plan 
#define INTERNA_TON_ISDN_NPI	((UInt8) (0x80 | (InternationalTON << 4) | ISDNNP))	///< International number and ISDN Numbering Plan 

/**
  	Maximum Facility IE length definition
**/
#define MAX_FACILITY_IE_LENGTH		251
  
//******************************************************************************
//
// Shared between MMI, MSC and MNCC 
//
//******************************************************************************
#define	SMS_MAX_DIGITS					40		///< Since this symbol MAX_DIGITS is used in SMS as well
												///< I created another symbol SMS_MAX_DIGITS with the same
												///< length in order not to break anything ...
												///< JAY MIGHT BE ABLE TO CONFIRM WHETHER IT IS SAFE TO
												///< USE THE SAME LENGTH OR NOT

#define	MAX_DIGITS						80		///< 04.08 Called Party BCD Number
												///< Max. number BCD Digits in a phone number
												///< 40 bytes in Packed BCD or 80 in unpacked BCD
												///< This value is limited by Network, so it
												///< should not be used for Phonebook telephone unless
												///< the Phonebook is also limited to this size.

#define MAX_PLMN_SEARCH					10		///< Maximum PLMN Search value
#define MAX_SIGNAL_STRENGTH_LEVEL		63		///< Max. Signal strength level 0x3f
#define	MAXNUM_CALLS					6 		///< Max. number of connected call allowed
#define PROTOCOLDISC_CC					0x03	///< CC protocol discriminator
#define PROTOCOLDISC_SS					0x0B	///< SS protocol discriminator
#define PHASE1_MAX_USSD_STRING_SIZE		200		///< Max. USSD string size for Phase1 (3GPP 24.080)
#define PHASE2_MAX_USSD_STRING_SIZE		160		///< Max. USSD string size for Phase2 (3GPP 24.080)
#define MAX_NAME_STRING_SIZE			160		///< Max. Calling Name string size


// String coding types according to 3GPP 3.38
#define STR_CODING_DEF_ALPHABET			0		// Default Alphabet (7bit)
#define STR_CODING_DEF_8bit				1		// 8bit Alphabet
#define STR_CODING_DEF_UCS2				2		// UCS2

/**
	ISO-639 language list in which each language is represented by a pair of 
	alpha-numberic characters, e.g. "EN" (0x454E) for English.
**/
#define ISO_AR_LANG   0x4152
#define ISO_CS_LANG   0x4353
#define ISO_DA_LANG   0x4441
#define ISO_DE_LANG   0x4445
#define ISO_EL_LANG   0x454C
#define ISO_EN_LANG   0x454E
#define ISO_ES_LANG   0x4553
#define ISO_FI_LANG   0x4649
#define ISO_FR_LANG   0x4652
#define ISO_HE_LANG   0x4845
#define ISO_HU_LANG   0x4855
#define ISO_IT_LANG   0x4954
#define ISO_NL_LANG   0x4E4C
#define ISO_NO_LANG   0x4E4F
#define ISO_PL_LANG   0x504C
#define ISO_PT_LANG   0x5054
#define ISO_RU_LANG   0x5255
#define ISO_SV_LANG   0x5356
#define ISO_TR_LANG   0x5452


/**
 CB language where each language is encoded as one byte: See section 5 of GSM 03.38
**/
#define CBS_GERMAN_LANG       0x00
#define CBS_ENGLISH_LANG      0x01
#define CBS_ITALIAN_LANG      0x02
#define CBS_FRENCH_LANG       0x03
#define CBS_SPANISH_LANG      0x04
#define CBS_DUTCH_LANG        0x05
#define CBS_SWEDISH_LANG      0x06
#define CBS_DANISH_LANG       0x07
#define CBS_PORTUGUESE_LANG   0x08
#define CBS_FINNISH_LANG      0x09
#define CBS_NORWEGIAN_LANG    0x0A
#define CBS_GREEK_LANG        0x0B
#define CBS_TURKISH_LANG      0x0C
#define CBS_HUNGARIAN_LANG    0x0D
#define CBS_POLISH_LANG       0x0E
#define CBS_UNSPECIFIED_LANG  0x0F
#define CBS_CZECH_LANG        0x20
#define CBS_HEBREW_LANG       0x21
#define CBS_ARABIC_LANG       0x22
#define CBS_RUSSIAN_LANG      0x23     


// CallIndex_t is used to identify a call in the MS.  A call is defined
// as an Active Speech/Data or Supplementary Service

/** 
	Call Index Values
**/

typedef enum
{
	CALLINDEX_00 = 0,							///< 1st Call
	CALLINDEX_01 = 1,							///< 2nd Call 
	CALLINDEX_02 = 2,							///< 3rd Call
	CALLINDEX_03 = 3,							///< 4th Call
	CALLINDEX_04 = 4,							///< 5th Call
	CALLINDEX_05 = 5,							///< 6th Call
	CALLINDEX_06 = 6,							///< 7th Call
	CALLINDEX_07 = 7,							///< 8th Call
	CALLINDEX_08 = 8,							///< 9th Call
	CALLINDEX_09 = 9,							///< 10th Call
	CALLINDEX_10 = 10,							///< 11th Call
	CALLINDEX_11 = 11,							///< 12th Call
	CALLINDEX_12 = 12,							///< 13th Call
	CALLINDEX_13 = 13,							///< 14th Call
	CALLINDEX_14 = 14,							///< 15th Call
	CALLINDEX_15 = 15,							///< 16th Call
	MAX_CALL_INDEX = 16,						///< Max. Call Index
	CALLINDEX_NO_CALL_TONE = 17,				///< Special Call Index, when no
												///< call exist, but a special Tone
												///< Generation is required.
	CALLINDEX_INVALID = 0xFF					///< Invalid Call Index
} CallIndex_t;				///< MMI<-->MSC<-->MNCC Call Identifier

/**
	Binary Coded Digit Number type
**/
typedef enum
{
	// BCDNUM_ZERO = 0					// Normal numbers are themselves
	//		thru						//
	// BCDNUM_NINE = 9					//
	BCDNUM_STAR = 10,					///< "*"
	BCDNUM_HASH = 11,					///< "#"
	BCDNUM_PAUSE = 12,					///< "p" 
	BCDNUM_WILD = 13,					///< "?"
	BCDNUM_B = 14,						///< "b"
	BCDNUM_END = 15						///< end of phone number;
} BCDNumber_t;

/* See Section 10.5.1 of GSM 11.11 */
#define DTMF_SEPARATOR_CHAR			'p'		/* Correspond to BCDNUM_PAUSE */
#define DTMF_SEPARATOR_UPPER_CHAR	'P'		/* Correspond to BCDNUM_PAUSE */
#define WILD_CARDING_CHAR			'?'		/* Correspond to BCDNUM_WILD */
#define EXPANSION_CHAR				'b'		/* Correspond to BCDNUM_B */



#define   CALLING_NAME_PRESENT      0x01	
#define   CALLING_NAME_AVAILABLE    0x02
#define   CALLING_NUMBER_PRESENT    0x10
#define   CALLING_NUMBER_AVAILABLE  0x20

/**
	Calling Name Presentation Masks
**/

typedef enum
{
  CALLING_NAME_NUMBER_RESTRICTED        = 0x00,
  CALLING_NAME_PRESENT_AND_AVAILABLE    = 0x03,
  CALLING_NUMBER_PRESENT_AND_AVAILABLE  = 0x30,
  CALLING_NAME_NUMBER_PRESENT_AVAILABLE = 0x33
} PresentStatus_t;

/// Telephone Number Type
typedef struct
{
	gsm_TON_t	ton;					///< Type Of Number
	gsm_NPI_t	npi;					///< Numbering Plan Identification
	UInt8		number[MAX_DIGITS+1];	///< ASCII representation of
										///< telephone number (non-BCD format) size
										///< of telephone number is determine by
										///< the size of the string
} TelephoneNumber_t;


#define MAX_BCD_NUMBER_SIZE				20						///< Max BCD Number Size
#define MAX_PHONE_NUMBER_SIZE			MAX_BCD_NUMBER_SIZE * 2	///< Max Phone Number Size
#define MAX_PHONE_SUBADDRESS_SIZE		21						///< Max Phone Subaddress Size

/**
Enum : Configuration Mode Type
**/
typedef enum
{
	CONFIG_MODE_DEFAULT					= 0x00,	///< Default
	CONFIG_MODE_INVOKE					= 0x01,	///< Invoke
	CONFIG_MODE_SUPPRESS				= 0x02	///< Supress
} ConfigMode_t;									///< Configuration Mode Type


/**
Enum : Type of Number
**/
typedef enum
{
	TON_UNKNOWN							= 0x00,	///< Unknown
	TON_INTERNATIONAL_NUMBER			= 0x01,	///< International Number
	TON_NATIONAL_NUMBER					= 0x02,	///< Netional Number
	TON_NETWORK_SPECIFIC_NUMBER			= 0x03,	///< Network Specific Number
	TON_DEDICATED_ACCESS_SHORT_CODE		= 0x04,	///< Dedicated Access Short Code 
	TON_RESERVED						= 0x05	///< Reserved
} TypeOfNumber_t;								///< Type of Number

/**
Enum : Number Plan ID
**/
typedef enum
{
	NPI_UNKNOWN							= 0x00,	///< Unknown
	NPI_ISDN_TELEPHONY_NUMBERING_PLAN	= 0x01,	///< ISDN/Telephony numbering plan
	NPI_DATA_NUMBERING_PLAN				= 0x03,	///< Data numbering plan
	NPI_TELEX_NUMBERING_PLAN			= 0x04,	///< Telex numbering plan
	NPI_NATIONAL_NUMBERING_PLAN			= 0x08,	///< National numbering plan
	NPI_PRIVATE_NUMBERING_PLAN			= 0x09	///< Private numbering plan
} NumberPlanId_t;								///< Numbering Plan Identification

/**
Enum : Type of Subaddress
**/
typedef enum
{
	TOS_NSAP							= 0x00,	///< NSAP (X.213/ISO 8348 AD2
	TOS_USER_SPECIFIED					= 0x01	///< User Specified
} TypeOfSubAdd_t;								///< Type Of Subaddress

/**
Enum : Odd Even Type
**/
typedef enum
{
	ODD_NUMBER_OF_DIGITS				= 0x00,	///< Odd Number Of Digits
	EVEN_NUMBER_OF_DIGITS				= 0x01	///< Even Number Of Digits
} OddEven_t;									///< Odd Even Type

/// Structure : Party Number
typedef struct
{
	TypeOfNumber_t	ton;								///< Type Of Number Type
	NumberPlanId_t	npi;								///< Number Plan Identification Type
	UInt8			length;								///< Length
	UInt8			number[MAX_PHONE_NUMBER_SIZE];		///< Phone Number
} PartyAddress_t;										///< Party Address

/// Structure : Party Sub Add
typedef struct
{
	TypeOfSubAdd_t	tos;								///< Type Of Subaddress
	OddEven_t		oddEven;							///< Odd Even Type
	UInt8			length;								///< Length
	UInt8			number[MAX_PHONE_SUBADDRESS_SIZE];	///< Phone Subaddress Number
} PartySubAdd_t;										///< Party Sub Address


/// Calling Name
typedef struct
{
	UInt8 data_cod_scheme;				///< Data Coding Schemes
	UInt8 length_in_char;				///< length
	UInt8 name_size;					///< Size of the name
	UInt8 name[MAX_NAME_STRING_SIZE+1]; ///< Array of the name
} CallingName_t;

/// Calling Information
typedef struct
{
   PresentStatus_t   present;		///< Call Line ID presentation
   CallingName_t	 name_info;		///< Calling name
   //TelephoneNumber_t number_info;
} CallingInfo_t;


#define SUB_LENGTH		 20		///< Length of sub address

/// Sub Address
typedef struct
{
	UInt8 		tos;
	UInt8 		odd_even;
	UInt8 		c_subaddr;
	UInt8 		subaddr[ SUB_LENGTH ];
} Subaddress_t;

/// USSD String Structure
typedef struct
{
	UInt8			used_size;
	UInt8			string[PHASE2_MAX_USSD_STRING_SIZE+1];
	UInt8			dcs;  // (dcs&0x0F)>>2: 0-Default Alphabet; 1-8bit; 2-UCS2
} USSDString_t;

#define	DTMF_TONE_ARRAY_SIZE 15	
#define	MNCC_DTMF_LIST_SIZE	300												///< The DTMF list size
//Note: MNCC_MAX_DTMF_TONE_DURATION shall not
//be bigger than MNCC_START_DTMF_TIMER_T336
#define MNCC_DTMF_TONE_TICK					(Ticks_t)   10							///<  10 milli seconds
#define MNCC_MIN_DTMF_TONE_DURATION			(Ticks_t)(  10 * MNCC_DTMF_TONE_TICK)	///< 100 milli seconds
#define MNCC_MAX_DTMF_TONE_DURATION			(Ticks_t)( 900 * MNCC_DTMF_TONE_TICK)	///< 9 Seconds
#define	MNCC_INVALID_DTMF_TONE_DURATION   	(Ticks_t)0						///< used for DTMF_START_STOP method only 
#define	MNCC_START_DTMF_TIMER_T336			(Ticks_t)(1000 * MNCC_DTMF_TONE_TICK)	///< T336 = 10 Seconds
#define	MNCC_STOP_DTMF_TIMER_T337			(Ticks_t)(1000 * MNCC_DTMF_TONE_TICK)	///< T337 = 10 Seconds

typedef UInt8 DTMFTone_t;				///< 0 - 9,
										///< A - D,
										///< STAR(*) = 15,
										///< HASH(#) = 16


/**
Enum :	DTMF Methode type
**/
typedef enum
{
	DTMF_METHOD_NONE,					///< No particular DTMF methode.
	DTMF_METHOD_START_ONLY,				///< If at the API level the client just calls CcApi_StartDtmf
	DTMF_METHOD_START_STOP				///< If at the API level the client insequence calls CcApi_StartDtmf/CcApiStopDtmf
} DtmfMethod_t;							///< DTMF methode type


/**
Enum :	DTMF Timer type
**/
typedef enum
{
	DTMF_TONE_DURATION_TYPE,			///< This timer is used for the duration of the DTMF tone.
	DTMF_WAIT_DURATION_TYPE,			///< This timer is used for the pause or wait duration (P/W)
	DTMF_PERIODIC_PAUSE_TYPE			///< This timer is used for the periodic pause between each tone.
} DtmfTimer_t;							///< DTMF timer type


/**
Enum :	DTMF Status type
**/
typedef enum
{
	DTMF_STATE_IDLE,					///< In this stat there is no DTMF activities
	DTMF_STATE_SEND_PEND,				///< In this stat the start DTMF is sent to the nw and waiting for NW ack
	DTMF_STATE_SEND,					///< In this state the ack is receive but we wait for tone duration timer to expire
	DTMF_STATE_STOP_PEND,				///< In this stat the stop DTMF is sent to the nw and waiting for NW ack
	DTMF_STATE_STOP						///< In this state we wait for periodically pause timer to expire.
} DtmfState_t;							///< DTMF State type


/// Structure :	API DTMF type
typedef struct
{
	UInt8		callIndex;				///< The call index of the call session
	UInt16		dtmfIndex;				///< The DTMF tone index
	DTMFTone_t	dtmfTone;				///< DTMF Tone
	UInt8		toneValume;				///< DTMF tone volume
	Boolean		isSilent;				///< Silent Flag
	Ticks_t		duration;				///< Duration of the DTMF tone [MiliSecond]
} ApiDtmf_t;							///< API DTMF type


/**
Enum : Coding group : "Data Codin Scheme GSM 3.38, section 5"
**/
typedef enum
{
	CODING_GROUP_0_LANGUAGE_GSM_7_BIT_ALPHABET,			///< German, English, Italian ....
	CODING_GROUP_1_MESSAGE_PROTECTED_BY_LANGUAGE_IND,	///< GSM 7 Bit Default
	CODING_GROUP_2_LANGUAGE_GSM_7_BIT_ALPHABET,			///< Czech, Hebrew, Arabic, ....
	CODING_GROUP_3_LANGUAGE_GSM_7_BIT_ALPHABET,			///< Languages using the GSM 7 bit default alphabet
	CODING_GROUP_4_GENERAL_TEXT_UNCOMPRESSED = 0x04,	///< General, Text Uncompressed
	CODING_GROUP_5_GENERAL_TEXT_UNCOMPRESSED,			///< General, Text Uncompressed, have message class meaning
	CODING_GROUP_6_GENERAL_TEXT_COMPRESSED,				///< General, Text Compressed
	CODING_GROUP_7_GENERAL_TEXT_COMPRESSED,				///< General, Text Compressed, have message class meaning
	CODING_GROUP_9_MSG_WITH_USER_DATA_HEADER = 0x09,	///< Message with User Data Header (UDH)
	CODING_GROUP_E_WAP_RELATED = 0x0E,					///< Refer to "Wireless Datagram Protocol Specification", Wireless Application Protocol Forum Ltd.
	CODING_GROUP_F_DATA_CODING_MSG_HANDLING				///< Data Coding/Message Handling
} CodingGroup_t;										///< SS Data Coding Scheme Type


/**
Enum :	Coding group 0: "Data Codin Scheme GSM 3.38, section 5"
		Coding group 0 : CODING_GROUP_0_LANGUAGE_GSM_7_BIT_ALPHABET
**/
typedef enum
{
	LANGUAGE_GERMAN,							///<  0. German
	LANGUAGE_ENGLISH,							///<  1. English
	LANGUAGE_ITALINA,							///<  2. Italian
	LANGUAGE_FRENCH,							///<  3. French
	LANGUAGE_SPANISH,							///<  4. Spanish
	LANGUAGE_DUTCH,								///<  5. Dutch
	LANGUAGE_SWEDISH,							///<  6. Swedish
	LANGUAGE_DANISH,							///<  7. Danish
	LANGUAGE_PORTUGUESE,						///<  8. Portuguese
	LANGUAGE_FINNISH,							///<  9. Finnish
	LANGUAGE_NORWEGIAN,							///< 10. Norwegian
	LANGUAGE_GREEK,								///< 11. Greek
	LANGUAGE_TURKISH,							///< 12. Turkish
	LANGUAGE_HUNGARIAN,							///< 13. Hungarian
	LANGUAGE_POLISH								///< 14. Polish
} CodingGroup0_t;								///< Coding Group 0 Type


/**
Enum :	Coding Croup 1: "Data Codin Scheme GSM 3.38, section 5"
		Coding group 1 : CODING_GROUP_1_MESSAGE_PROTECTED_BY_LANGUAGE_IND

		0000	Default alphabet; message preceded by language indication. 
				The first 3 characters of the message are a two-character
				representation of the language encoded according to ISO 639 [12],
				followed by a CR character. The CR character is then followed by
				90 characters of text. A Pre-Phase 2+ MS will overwrite the start
				of the message up to the CR and present only the text.

		0001	UCS2; message preceded by language indication
				The message starts with a two 7-bit default alphabet character
				representation of the language encoded according to ISO 639 [12].
				This is padded to the octet boundary with two bits set to 0 and
				then followed by 40 characters of UCS2-encoded message.

				An MS not supporting UCS2 coding will present the two character
				language identifier followed by improperly interpreted user data.

		0010..1111	Reserved for European languages
**/
typedef enum
{
	GSM_7_BIT_DEFAULT_ALPHABET,					///< GSM 7 bit default alphabet
	UNIVERSAL_CHARACTER_SET_2					///< UCS2
} CodingGroup1_t;								///< Coding Group 0 Type


/**
Enum :	Coding Croup 2: "Data Codin Scheme GSM 3.38, section 5"
		Coding group 2 : CODING_GROUP_2_LANGUAGE_GSM_7_BIT_ALPHABET

		0000	Czech
		0001	Hebrew
		0010	Arabic
		0011	Russian
		0100	Icelandic

		0001 .. 1111 Reserved for European Languages using the default alphabet,
				with unspecified handling at the MS
**/
typedef enum
{
	LANGUAGE_CZECH,								///< Czech
	LANGUAGE_HEBREW,							///< Hebrew
	LANGUAGE_ARABIC,							///< Arabic
	LANGUAGE_RUSSIAN,							///< Russian
	LANGUAGE_ICELANDIC							///< Icelandic
} CodingGroup2_t;								///< Coding Group 2 Type


/**
Enum :	Message Class: "Data Coding Scheme GSM 3.38, section 5"
@code
		Coding group 4 : CODING_GROUP_4_GENERAL_TEXT_UNCOMPRESSED
		Coding group 5 : CODING_GROUP_5_GENERAL_TEXT_UNCOMPRESSED
		Coding group 6 : CODING_GROUP_6_GENERAL_TEXT_COMPRESSED
		Coding group 7 : CODING_GROUP_7_GENERAL_TEXT_COMPRESSED
		Coding group 9 : CODING_GROUP_9_MSG_WITH_USER_DATA_HEADER
		Coding group F : CODING_GROUP_F_DATA_CODING_MSG_HANDLING
@endcode
**/
typedef enum
{
	MESSAGE_CLASS_0,							///< Class 0
	MESSAGE_CLASS_1,							///< Class 1 ME Specific/User Defined
	MESSAGE_CLASS_2,							///< Class 2 (U)SIM Specific/User Defined
	MESSAGE_CLASS_3								///< Class 3 TE-Specific(3GPP TS 27.005)
} MessageClass_t;								///< Message Class


/**
Enum :	Alphabet: "Data Coding Scheme GSM 3.38, section 5"
@code
		Coding group 4 : CODING_GROUP_4_GENERAL_TEXT_UNCOMPRESSED
		Coding group 5 : CODING_GROUP_5_GENERAL_TEXT_UNCOMPRESSED
		Coding group 6 : CODING_GROUP_6_GENERAL_TEXT_COMPRESSED
		Coding group 7 : CODING_GROUP_7_GENERAL_TEXT_COMPRESSED
		Coding group 9 : CODING_GROUP_9_MSG_WITH_USER_DATA_HEADER
@endcode
**/
typedef enum
{
	CHARACTER_SET_GSM_7_BIT_DEFAULT,			///< GSM 7 bit default alphabet
	CHARACTER_SET_8_BIT_DATA,					///< 8 bit data
	CHARACTER_SET_UCS2_16_BIT,					///< UCS2 16 bit
	CHARACTER_SET_RESERVED						///< Reserved
} CharacterSet_t;								///< Character Set


/**
Enum :	Message Coding: "Data Codin Scheme GSM 3.38, section 5"
		Coding group F : CODING_GROUP_F_DATA_CODING_MSG_HANDLING
**/
typedef enum
{
	MESSAGE_CODING_GSM_7_BIT_DEFAULT_ALPHABET,	///< GSM 7 bit default alphabet
	MESSAGE_CODING_GSM_8_BIT_DATA				///< GSM 8 bit data
} MsgCoding_t;									///< Message Coding Type



typedef struct
{
	ApiDtmf_t	dtmfObj;				///< DTMF object
	DtmfState_t	dtmfState;				///< DTMF state
	Result_t	cause;					///< Cause of the event
} ApiDtmfStatus_t;						///< API DTMF status type

typedef UInt32 Duration_t;				///< duration of call in Seconds
typedef UInt32 CallMeterUnit_t;			///< General Call Meter Unit
typedef CallMeterUnit_t ACM_t;			///< Accumulated Call Meter


/**
	Registration status of GSM and GPRS services
**/
 
typedef enum
{
	REGISTERSTATUS_NORMAL,			///< (GSM) Normal service
									///< (GPRS) GPRS service available and attached
	REGISTERSTATUS_LIMITED,			///< (GSM) Emergency calls only
									///< (GPRS) GPRS service available but detached
	REGISTERSTATUS_NO_ACCESS,		///< (GSM) No access (non-fatal reject from network or lower layer failure)
									///< (GPRS) N/A
	REGISTERSTATUS_NO_CELL,			///< (GSM/GPRS) No cell (temporary loss of network)
	REGISTERSTATUS_NO_SERVICE,		///< (GSM) N/A
									///< (GPRS) GPRS service not available
	REGISTERSTATUS_NO_NETWORK,		///< (GSM/GPRS) No Network available
	REGISTERSTATUS_INVALID_SIM,		///< (GSM/GPRS) Invalid SIM
	REGISTERSTATUS_INVALID_ME,		///< (GSM/GPRS) Illegal ME or MS
	REGISTERSTATUS_SERVICE_DISABLED, ///< (GSM/GPRS) Service disabled by higher layer (SIM detach, power down, detach req)
									 ///<            or by network (network initiated detach)
	REGISTERSTATUS_NO_STATUS,		///< (GSM/GPRS) No status info; it can be ignored

	REGISTERSTATUS_NOT_REG_SEARCHING = 0xFF
} RegisterStatus_t;					///< MSC-->MMI : Registration status


// **FixMe** this one will evantually be replaced by the next enum type
//typedef enum
//{
//	NETREGSTAT_NOT_REG_NOT_SEARCHING,	// Not registered, ME not currently searching a new operator
//	NETREGSTAT_REG_HOME,				// Registered, home network
//	NETREGSTAT_NOT_REG_SEARCHING,		// Not register, ME currently searching a new operator
//	NETREGSTAT_REG_DENIED,				// Registration denied
//	NETREGSTAT_UNKNOWN,					// Unknown
//	NETREGSTAT_REG_ROAMING				// Registered, roaming
//} NetworkRegisterStatus_t;				// Network Registration Status; see GSM 07.07, 7.2 and 10.1.13

/**
	Mobile Registration States
**/

typedef enum {
	REG_STATE_NO_SERVICE,			///< GPRS service is not available.
	REG_STATE_NORMAL_SERVICE,		///< Mobile is in normal service, GPRS attached
	REG_STATE_SEARCHING,			///< Mobile is searching for network to camp.Services are not yet available.
	REG_STATE_LIMITED_SERVICE,		///< Mobile is in limited service - only Emergency calls are allowed. GPRS is available but not attached.	
	REG_STATE_UNKNOWN,				///< Unknown state. 
	REG_STATE_ROAMING_SERVICE,		///< Mobile is roaming. 
	REG_STATE_NO_CHANGE				///< No change in state from previously reported.
} MSRegState_t;


/// PLMN structure
typedef struct
{
	UInt16 mcc;			///< Mobile Country Code (3-digits) and may include the 3rd digit of MNC (Example: 0x13F0 or 0x1300 for Cingular)
	UInt8  mnc;			///< Mobile Network Code (2-digits) (Example: 0x71 for Cingular)
	UInt8  order;		///< the order number of operator in SIM preferred operator list
} PLMN_ID_t;
// PLMN Identification

/**
	Call Type
*/
typedef enum
{
	CALLTYPE_SPEECH,					///< Call is a Speech Call
	CALLTYPE_SUPPSVC,					///< Call is a Call-Independent Supplementary Service
	CALLTYPE_DATA,						///< Call is a Data Call, Data
	CALLTYPE_FAX,						///< Call is a Data Call, Fax
	CALLTYPE_MOUSSDSUPPSVC,				///< Call is a MO USSD Service
	CALLTYPE_MTUSSDSUPPSVC,				///< Call is a MT USSD Service
	CALLTYPE_GPRS,						///< Call is a GPRS call
	CALLTYPE_PPP_LOOPBACK,				///< Call is a PPP loopback test
	CALLTYPE_VIDEO,						///< Call type Video - used by Wedge
	CALLTYPE_UNKNOWN					///< Call type is unknown
} CallType_t;
/**
	Call status
*/
typedef enum
{
	CALLSTATUS_CALLING,					///< 00, MO Call is Calling
	CALLSTATUS_ACTIVE,					///< 01, a Call is Active
	CALLSTATUS_HELD,					///< 02, a Call is Held
	CALLSTATUS_HANGUP,					///< 03, a Call is Hangup
	CALLSTATUS_MO_CI_ALLOC,				///< 04, Allocated a MO CallIndex
	CALLSTATUS_MT_CI_ALLOC,				///< 05, Allocated a MT CallIndex

	CALLSTATUS_ERR_CALL_BARRED,			///< 06, Error, Call is Barred (not used)
	CALLSTATUS_ERR_NETWORK_BUSY,		///< 07, Error, Network is Busy (not used)
	CALLSTATUS_ERR_NO_SERVICE,			///< 08, Error, No Service is Available (not used)
	CALLSTATUS_ERR_NO_NETWORK,			///< 09, Error, No Network

	CALLSTATUS_IDLE,					///< 0A,
	CALLSTATUS_ALERTING,				///< 0B, MO Call, Network is Alerting destination
	CALLSTATUS_MPTY_ACTIVE,				///< 0C, MPTY Call is Active
	CALLSTATUS_MPTY_HELD,				///< 0D, MPTY Call is Held 
	CALLSTATUS_CONNECTED,				///< 10, MO Call Connected, but not Active
	CALLSTATUS_VERIFIED,				///< 11, MO Call, UserId and Password is verified
	CALLSTATUS_USER_BUSY				///< 12, MO Call, Called party busy

} CallStatus_t;							///< MSC-->MMI : call status

/**
	Call Action
*/

typedef enum
{
	CALLACTION_ACCEPT,					///<  00, Accept the a Call
	CALLACTION_HANGUP,					///<  01, Hangup a Call
	CALLACTION_HOLD,					///<  02, Hold Active Call
	CALLACTION_RETRIEVE,				///<  03, Retrieve a Held Call
	CALLACTION_JOIN,					///<  04, Join two Calls
	CALLACTION_SPLIT,					///<  05, Split a Joined Call

	CALLACTION_SWAP,					///<  06, Swap an Active Call with a Held Call
	CALLACTION_END,						///<  07, End All Calls
	CALLACTION_ENDMPTY,					///<  08, End MPTY call
	CALLACTION_ECT,						///<  09, Explicit Call Transfer
	CALLACTION_START_DTMF_0,			///<  0A, Start generating DTMF Tone for 0
	CALLACTION_START_DTMF_1,			///<  0B, Start generating DTMF Tone for 1
	CALLACTION_START_DTMF_2,			///<  0C, Start generating DTMF Tone for 2
	CALLACTION_START_DTMF_3,			///<  0D, Start generating DTMF Tone for 3
	CALLACTION_START_DTMF_4,			///<  0E, Start generating DTMF Tone for 4
	CALLACTION_START_DTMF_5,			///<  0F, Start generating DTMF Tone for 5
	CALLACTION_START_DTMF_6,			///<  10, Start generating DTMF Tone for 6
	CALLACTION_START_DTMF_7,			///<  11, Start generating DTMF Tone for 7
	CALLACTION_START_DTMF_8,			///<  12, Start generating DTMF Tone for 8
	CALLACTION_START_DTMF_9,			///<  13, Start generating DTMF Tone for 9
	CALLACTION_START_DTMF_POUND,		///<  14, Start generating DTMF Tone for #
	CALLACTION_START_DTMF_STAR,			///<  15, Start generating DTMF Tone for *
	CALLACTION_STOP_DTMF,				///<  16, Stop generating DTMF Tone
	CALLACTION_START_DIAL_TONE,			///<  17, Start generating Dial Tone
	CALLACTION_STOP_DIAL_TONE,			///<  18, Stop generating Dial Tone
	CALLACTION_START_DTMF_0_SILENT,		///<  19, Start generating DTMF Tone for 0 but silent on speaker
	CALLACTION_START_DTMF_1_SILENT,		///<  1A, Start generating DTMF Tone for 1 but silent on speaker
	CALLACTION_START_DTMF_2_SILENT,		///<  1B, Start generating DTMF Tone for 2 but silent on speaker
	CALLACTION_START_DTMF_3_SILENT,		///<  1C, Start generating DTMF Tone for 3 but silent on speaker
	CALLACTION_START_DTMF_4_SILENT,		///<  1D, Start generating DTMF Tone for 4 but silent on speaker
	CALLACTION_START_DTMF_5_SILENT,		///<  1E, Start generating DTMF Tone for 5 but silent on speaker
	CALLACTION_START_DTMF_6_SILENT,		///<  1F, Start generating DTMF Tone for 6 but silent on speaker
	CALLACTION_START_DTMF_7_SILENT,		///<  2A, Start generating DTMF Tone for 7 but silent on speaker
	CALLACTION_START_DTMF_8_SILENT,		///<  2B, Start generating DTMF Tone for 8 but silent on speaker
	CALLACTION_START_DTMF_9_SILENT,		///<  2C, Start generating DTMF Tone for 9 but silent on speaker
	CALLACTION_START_DTMF_POUND_SILENT,	///<  2D, Start generating DTMF Tone for # but silent on speaker
	CALLACTION_START_DTMF_STAR_SILENT, 	///<  2E, Start generating DTMF Tone for * but silent on speaker
	CALLACTION_START_RING_TONE,			///<  2F, start ringing tone
	CALLACTION_STOP_RING_TONE,			///<  30, stop ringing tone
	CALLACTION_NONE						///<  31, None
} CallAction_t;							///<  MMI-->MSC : call action

/**
	Data Call Mode
**/
typedef enum
{
	DATACALLMODE_DISALLOW,				///<  Data Calls are Disallowed
	DATACALLMODE_ALLOW					///<  Data Calls are Allowed
} DataCallMode_t;


#define	CUGINDEX_NONE		0xFFFF		///<  Defines no CUG index
#define	MAX_CUGINDEX		0x7FFF		///<  Defines Maximum value for CUG index
#define	MIN_CUGINDEX		0x0000		///<  Defines Minimum value for CUG index
typedef UInt16 CUGIndex_t;				///<  CUG Index value

/**
	Closed user Group Suppress Preference
**/

typedef enum
{
	CUGSUPPRESSPREFCUG_DISABLE,			///<  Suppress Preferential CUG Indicator not sent
	CUGSUPPRESSPREFCUG_ENABLE			///<  Suppress Preferential CUG Indicator sent
} CUGSuppressPrefCUG_t;

/**
	CUG Suppress
**/

typedef enum
{
	CUGSUPPRESSOA_DISABLE,				///<  Suppress OA Indicator not sent
	CUGSUPPRESSOA_ENABLE				///<  Suppress OA Indicator sent
} CUGSuppressOA_t;

/// Closed User Group Information
typedef struct
{
	CUGIndex_t cug_index;					///<  CUG Index value
	CUGSuppressPrefCUG_t suppress_pref_cug; ///<  Suppress Preferential CUG Indicator
	CUGSuppressOA_t suppress_oa;			///<  Suppress Outside Access Indicator
} CUGInfo_t;								///<  Closed User Group (CUG)

/**
	Caller Line ID Restriction
*/
typedef enum
{
	CLIRMODE_DEFAULT,					///<  Default Caller Line Indentification
	CLIRMODE_INVOKED,					///<  CLIR Invoked, Called Party doesn't see Calling Party's CLI
	CLIRMODE_SUPPRESSED					///<  CLIR Suppressed, Called Party see Calling Party's CLI
} CLIRMode_t;

#define MAX_BEARER_CAP_LEN  16
#define MAX_SUPPORTED_CODEC_LIST_LEN 10		///< currently only two technologies are supported, we will have 4 octets for each technology , one for IEI , one for length of the IE

/// Bearer Capability of a data call
typedef struct
{
	UInt8			val[MAX_BEARER_CAP_LEN];  ///< Release 99 BC could be upto 16 bytes.
} BearerCapability_t;

/**
Enum:	This enum is used for repeat indicator description. 
		refer 3gpp 24.008 section 10.5.4.22
*/
typedef enum 
{
	RI_NOT_PRESENT	= 0,
	RI_CIRCULAR		= 1, 
	RI_SEQUENTIAL	= 3
} RepeatInd_t;

/**
Structure:	Bearer capability structure used to notify the bearer capability 
			used for the call
*/
typedef struct
{
	BearerCapability_t	Bc1;			///< bearercapability1  
	BearerCapability_t	Bc2;			///< bearercapability2
	RepeatInd_t 		RepeatInd;		///< repeat indicator
}CC_BearerCap_t;						///< Structure containing bearer capability related parameters.


/**
	Supplementary Service Type
**/	

typedef enum
{
	SUPPSVCTYPE_NONE,					///< 00, no SS
	SUPPSVCTYPE_CLIP,					///< 01, Calling Line ID Presentation
	SUPPSVCTYPE_CLIR,					///< 02, Calling Line ID Restriction
	SUPPSVCTYPE_COLP,					///< 03, Connected Line ID Presentation
	SUPPSVCTYPE_COLR,					///< 04, Connected Line ID Restriction
	SUPPSVCTYPE_CFU,					///< 05, Call Forwarding Unconditional
	SUPPSVCTYPE_CFB,					///< 06, Call Forwarding when Busy
	SUPPSVCTYPE_CFNRY,					///< 07, Call Forwarding on No Reply
	SUPPSVCTYPE_CFNRC,					///< 08, Call Forwarding on Not Reachable
	SUPPSVCTYPE_CF_ALL,					///< 09, Call Forwarding All
	SUPPSVCTYPE_CF_ALL_COND,			///< 0A, Call Forwarding All on Conditional
	SUPPSVCTYPE_CW,						///< 0B, Call Waiting
	SUPPSVCTYPE_BAR_PASSWORD,			///< 0C, Password Change requested
	SUPPSVCTYPE_BAOC,					///< 0D, Barring of All Outgoing Calls
	SUPPSVCTYPE_BOIC,					///< 0E, Barring Outgoing International Calls
	SUPPSVCTYPE_BOIC_EX_HC,				///< 0F, Barring Outgoing International Calls Excluding Home PLMN Country
	SUPPSVCTYPE_BAIC,					///< 10, Barring All Incoming Calls
	SUPPSVCTYPE_BAIC_ROAM,				///< 11, Barring All Incoming Calls when Roaming outside Home PLMN Country
	SUPPSVCTYPE_BAC,					///< 12, Barring of All Calls
	SUPPSVCTYPE_BOC,					///< 13, Barring of Outgoing Calls
	SUPPSVCTYPE_BIC,					///< 14, Barring of Incoming Calls
	SUPPSVCTYPE_ECT,					///< 15, Explicit Call Transfer
	SUPPSVCTYPE_USSD,					///< 16, Unstructure Supplementary Service Data

	SUPPSVCTYPE_NHOLD,					///< 17, Network Status of a Call on Hold
	SUPPSVCTYPE_NRETRIEVE,				///< 18, Network Status of a Retrieve Call
	SUPPSVCTYPE_CNAP					///< 19, Calling NAme Presentation

} SuppSvcType_t;						///< MSC-->MMI : call-indep. service type

/// Supplementary Service Status
typedef struct
{
	SuppSvcType_t	ssSvcType;
	UInt8			cfgParamValue;
} SS_IntParSetInd_t;

typedef enum
{
	SUPPSVCSTATUS_DEACTIVATED,				///< Service deactivation succeeded
	SUPPSVCSTATUS_ACTIVATED,				///< Service activation succeeded
	SUPPSVCSTATUS_UNKNOWN_SUBSCRIBER,		///< Unknown subscriber, action did not succeed.
	SUPPSVCSTATUS_ILLEGAL_SUBSCRIBER,		///< Illegal subscriber - service not available
	SUPPSVCSTATUS_BS_NOTPROVISIONED,		///< Bearer Service requested is not proviosned, request failed.
	SUPPSVCSTATUS_TS_NOTPROVISIONED,		///< Tele Service requested is not proviosned, request failed.
	SUPPSVCSTATUS_ILLEGAL_EQUIPMENT,		///< Illegal ME / SIM.
	SUPPSVCSTATUS_CALL_BARRED,				///< This type of call has been barred
	SUPPSVCSTATUS_ILLEGAL_SSOPERATION,		///< Service failed - illegal SS operation.
	SUPPSVCSTATUS_SS_ERRORSTATUS,			///< Returned error status - request failed.
	SUPPSVCSTATUS_NOT_AVAILABLE,		    ///< Service isn't available
	SUPPSVCSTATUS_SS_SUBSCRIPTION_VIOLATION, ///< This service has not been subscribed.
	SUPPSVCSTATUS_SS_INCOMPATIBILITY,		///< Service incompatible, request failed.
	SUPPSVCSTATUS_FACILITY_NOT_SUPPORTED,	///< Requested facility is not supported
	SUPPSVCSTATUS_ABSENT_SUBSCRIBER,		///< Subscriber is absent
	SUPPSVCSTATUS_SHORTTERM_DENIAL,			///< Short term denial of service, need to try again later.
	SUPPSVCSTATUS_LONGTERM_DENIAL,			///< Long term denial of service
	SUPPSVCSTATUS_SYSTEM_FAILURE,			///< System failure
	SUPPSVCSTATUS_DATA_MISSING,				///< Data is incomplete in the request
	SUPPSVCSTATUS_UNEXPECTED_DATA_VALUE,	///< Unexpected data in the request. Check request and try again.
	SUPPSVCSTATUS_PW_REGISTRATION_FAILURE,	///< Password registration failure
	SUPPSVCSTATUS_NEGATIVEPW_CHECK,			///< Password check negative
	SUPPSVCSTATUS_NUMBEROFPW_ATTEMPT_VIOLATION,///< Exceeded allowed number of password check attempts.
	SUPPSVCSTATUS_UNKNOWN_ALPHABET,			///< Unknown alphabet in request
	SUPPSVCSTATUS_USSD_BUSY,				///< Unstructured SS Data busy.
	SUPPSVCSTATUS_REJECTED_BY_USER,			///< Request/result rejected by user.
	SUPPSVCSTATUS_REJECTED_BY_NETWORK,		///< Request rejected by network
	SUPPSVCSTATUS_DEFLECTION_TO_SERVED_SUBSCRIBER,///< Service deflection to served subscriber.
	SUPPSVCSTATUS_SPECIAL_SERVICE_CODE,		///< Special Service Code
	SUPPSVCSTATUS_INVALID_DEFLECTED_TO_NUMBER,///< Call deflected to an invalid number
	SUPPSVCSTATUS_MAX_NUMBER_OF_MPTY_EXCEEDED,///< Exceeded the number of multi party calls
	SUPPSVCSTATUS_RESOURCE_NOT_AVAILABLE,	///< Resources are unavailable
	SUPPSVCSTATUS_FAILED,					///< Service request failed
	SUPPSVCSTATUS_USSD_RESEND,				///< Resend USSD request.
	SUPPSVCSTATUS_ABORT,					///< Abort on going request
 	SUPPSVCSTATUS_POSITION_METHODE_FAILURE,	///< Position method failure
 	SUPPSVCSTATUS_NBR_SB_EXCEEDED,			///< Number of subscribers exceeded
	SUPPSVCSTATUS_NULL						///< NULL Supplementary service
} SuppSvcStatus_t;							///< MSC-->MMI : call-indep. service result

/**
	Supplementary Service Codes
**/	

typedef enum
{
	SUPPSVCBSCODE_ALL_BEARER_TELESERVICE 		= 0,///< not specified in GSM 2.30 Spec.
													// but 'C' needs a value
	SUPPSVCBSCODE_NOT_DECODED					= 1,///< Basic Service was not decoded
													// use 'bs_raw_list' value todetermine the Basic Service
	SUPPSVCBSCODE_ALL_TELESERVICE 				= 10,///< All Teleservices(speech,fax, SMS,data) 
	SUPPSVCBSCODE_ALL_SPEECH 					= 11,///< All services specific to speech calls
	SUPPSVCBSCODE_ALL_DATA_TELESERVICE 			= 12,///< All Services specific to Data calls
	SUPPSVCBSCODE_ALL_FAX_SERVICE 				= 13,///< All services specific to Fax calls
	SUPPSVCBSCODE_ALL_SHORT_MESSAGE_SERVICE 	= 16,///< All services specific to SMS
	SUPPSVCBSCODE_ALL_TELESERVICE_EXCEPT_SMS 	= 19,///< All teleservices except SMS(speech,data,fax)
	SUPPSVCBSCODE_ALL_BEARER_SERVICE 			= 20,///< All Bearer services( Data and Packet services)
	SUPPSVCBSCODE_ALL_ASYNC_SERVICE 			= 21,///< All asynchronous services
	SUPPSVCBSCODE_ALL_SYNC_SERVICE 				= 22,///< All synchronous services
	SUPPSVCBSCODE_ALL_DATA_CIRCUIT_SYNC 		= 24,///< Data Services synchronous(General and Data circuit duplex)
	SUPPSVCBSCODE_ALL_DATA_CIRCUIT_ASYNC 		= 25,///< Data Services asynchronous(General and Data circuit duplex)
	SUPPSVCBSCODE_ALL_DEDICATED_PACKET_ACCESS 	= 26,///< Dedicated Packet services(Data Packet duplex and General Packet Service) 
	SUPPSVCBSCODE_ALL_DEDICATED_PAD_ACCESS 		= 27,///< Genral PAD access and all PAD data services
	SUPPSVCBSCODE_ALL_DATA_CDA_SERVICES 		= 28,///< All Data CDA Services
	SUPPSVCBSCODE_ALL_DATA_CDS_SERVICES 		= 29,///< All Data CDS Services
	SUPPSVCBSCODE_ALL_ALT_SPEECH_DATA_CDA_SRVS 	= 30,///< All Alternate Speech Data CDA Services
	SUPPSVCBSCODE_ALL_ALT_SPEECH_DATA_CDS_SRVS	= 31,///< All Alternate Speech Data CDS Services
	SUPPSVCBSCODE_ALL_SPEECH_FOLLOW_BY_DATA_CDA = 32,///< All Speech Followed By Data CDA Services
	SUPPSVCBSCODE_ALL_SPEECH_FOLLOW_BY_DATA_CDS = 33,///< All Speech Followed By Data CDS Services

	// See GSM 2.30 for all PLMN Spec. values
	SUPPSVCTSCODE_ALL_PLMN_SPECIFIC_TELESERVICE = 50,	///< PLMN specific Tele Services. See GSM 2.30
	SUPPSVCBSCODE_ALL_PLMN_SPECIFIC_BEARER_SERVICE = 70,///< PLMN specific Bearer Services. See GSM 2.30

	/* Teleservice code for ALS (Alternative Line Service) added to GSM 02.30, see Appendix 1 of CPHS spec */
	SUPPSVCBSCODE_ALL_AUXILIARY_TELEPHONY = 89///< Auxillary telephony
} SuppSvcBSCode_t;							


/// SS Basic Service Codes
typedef struct
{
	Boolean is_used;			///< TRUE if the user specifies a Supplementary Service class
	SuppSvcBSCode_t bs_code;	///< Basic code corresponding to the Supplementary Service class 
								///< selected, relevant only if "is_used" is TRUE. 
} SuppSvcUsedBSCode_t;

/**
	SS Basic Service type
**/
typedef enum
{
	SUPPSVCBSTYPE_ALL_SERVICE = 0,		///< All Services
	SUPPSVCBSTYPE_BEARER_SERVICE = 2,	///< Bearer Services(Data and Packet services) 
	SUPPSVCBSTYPE_TELESERVICE = 3		///< Tele Services(Speech,data,fax,SMS)
} SuppSvcBSType_t;

typedef UInt8 RawSSStatus_t;				///< Raw SS Status Type

/// SS Basic Services Raw Codes
typedef struct
{
	SuppSvcBSType_t bs_type;		///< Supplementary Services Bearer Services Type
	UInt8 bs_value;					///< Bearer Services Value
} SuppSvcBSRawCode_t;

/// Basic Services Group 
typedef struct
{
	UInt8 list_sz;							///< specifies the number of valid entries in list
	SuppSvcBSCode_t bs_decoded_list[13];	///< Decoded list of entries
	SuppSvcBSRawCode_t bs_raw_list[13];		///< Raw list as returned by the network
} BasicServiceGroupList_t;

/// Call forward feature
typedef struct
{
	SuppSvcStatus_t		ss_status;			///< Supplementary Service Status		
	RawSSStatus_t 		raw_ss_status;		///< Supplementary Service Raw Status
	SuppSvcBSCode_t 	bs_decoded;			///< Decoded Bearer Service Values
	SuppSvcBSRawCode_t 	bs_raw;				///< Raw bearer values
	TelephoneNumber_t 	forwarded_to_number;///< Number to which the call has been forwarded
	UInt8				noReplyTime;		///< 5 <= No Reply time <= 30
} FFeature_t;

/// Call forward feature list
typedef struct
{
	UInt8 list_sz;					///< specifies the number of valid entries in list
	Boolean bs_code_used;			///< True if the call forward request specifies a specific Service Class
	FFeature_t ffeature_list[13];	///< Call fwd feature
} FFeatureList_t;

/**
	Caller ID Restriction 
**/

typedef enum
{
	CLIRESTRICTIONOPTION_PERMANENT = 0,						///< Calling Line ID Restriction Option Permamnent
	CLIRESTRICTIONOPTION_TEMPORARY_DEFAULT_RESTRICTED = 1,	///< CLIR temporary mode presentation restricted
	CLIRESTRICTIONOPTION_TEMPORARY_DEFAULT_ALLOWED = 2		///< CLIR temporary mode presentation allowed
} CLIRestrictionOption_t;

/// Calling Line ID Restriction Information Structure
typedef struct
{
	Boolean					isCliRestOpt;		///< CLIR mode provisioned - TRUE / FALSE
	CLIRestrictionOption_t 	cli_restriction_opt;
	RawSSStatus_t 			raw_ss_status;		///< Raw SS status
} CLIRestrictionInfo_t;

/**
	USSD Service
**/
typedef enum
{
	USSD_REQUEST,							///< USSD Request
	USSD_NOTIFICATION,						///< USSD Notification
	USSD_FACILITY_RETURN_RESULT,			///< USSD Requested facility returned a result
	USSD_FACILITY_RETURN_ERROR,				///< USSD Request returns an Error
	USSD_FACILITY_REJECT,					///< USSD facility rejected. Check request
	USSD_RELEASE_COMPLETE_RETURN_RESULT,	///< Request processed with a result, release is complete.
	USSD_RELEASE_COMPLETE_RETURN_ERROR,		///< Release complete, returning an error
	USSD_RELEASE_COMPLETE_REJECT,			///< Facility rejected, release complete
	USSD_RESEND,							///< Request to re-send USSD request
	USSD_REGISTRATION						///< Register USSD request
}USSDService_t;

/// USSD Data
typedef struct
{
	USSDService_t	service_type;	///< USSD service type

	UInt8			oldindex;		///<Old USSD index, used for RESEND
	UInt8			newindex;		///< New USSD index

	UInt8			prob_tag;		///<for REJ
	UInt8			prob_code;		 

	SuppSvcStatus_t	err_code;		///<Error status

	Unicode_t		code_type;		///<UNICODE_GSM & UNICODE_UCS1: nonunicode, UNICODE_80: unicode
	UInt8			dcs;			///<Data Coding Scheme
	UInt8			used_size;		///<Length of the data payload
	UInt8			string[PHASE1_MAX_USSD_STRING_SIZE+1];  ///< USSD data buffer
}USSDData_t;

/**
	Supplementary Service Information
**/
typedef enum
{
	SSINFOUSE_DO_NOT_USE,					///< don't use the 'ss_info' union, data is invalid
	SSINFOUSE_RAW_SS_STATUS,				///< use 'raw_ss_status' in 'ss_info' union
	SSINFOUSE_FORWARDED_TO_NUMBER,			///< use 'forward_to_number' in 'ss_info' union
	SSINFOUSE_BS_GROUP_LIST,				///< use 'bs_group_list' in 'ss_info' union
	SSINFOUSE_FFEATURE_LIST,				///< use 'ffeature_list' in 'ss_info' union
	SSINFOUSE_CLI_RESTRICTION_INFO,			///< use 'cli_restriction_info' in 'ss_info' union
	SSINFOUSE_USSD_DATA,					///< use 'ussd_data' in 'ss_info' union
	SSINFOUSE_CALL_BARRING					///< Call Barring
} SSInfoUse_t;

/// Call Barring Information
typedef struct
{
	SuppSvcBSCode_t	basicSvcCode;			///< Basic Service Code
	UInt8			ssStatus;				///< Supplementary Service Status

} SS_CallBarringInfo_t;

/// Call Barring List
typedef struct
{	
	UInt8					size;			///< Call Barring list size
	SS_CallBarringInfo_t	info[13];		///< Call Barring Information

} SS_CallBarringList_t;

/// Supplementary Service Parameters
typedef struct
{
	// Following 'union' structure is only valid if 'ss_info_use' is not set to 'SSINFO_DO_NOT_USE'.
	// If 'ss_info_use' is set to 'SSINFO_DO_NO_USE', 'ss_info' contains invalid settings.
	SSInfoUse_t		ss_info_use;
	union 
	{
		FFeatureList_t 			ffeature_list;	///< Call forward feature list
		SS_CallBarringList_t	callBarList;	///< Call Barring information list
		RawSSStatus_t			raw_ss_status;	///< Raw(un decoded) SS information
		TelephoneNumber_t 		forwarded_to_number;///< Forwarded to telephone number
		BasicServiceGroupList_t bs_group_list;	///< Basic Services Group list
		CLIRestrictionInfo_t 	cli_restriction_info;  ///< Caller Line ID Restriction information
		USSDData_t				ussd_data_info;	///< USSD Data
	} ss_info;
} SuppSvcParam_t;			///< MSC-->MMI : call-indep. service result

/// USSD Data Information
typedef struct
{
	CallIndex_t call_index;		///< Call index associated with the USSD session
	USSDData_t ussd_data;		///< USSD data
} USSDataInfo_t;

/**
Connected Line Presentation
*/
typedef enum
{
	COLP_PRESENT_ALLOWED,				///< connected line id present
	COLP_PRESENT_RESTRICTED				///< connected line id not present
}COLPPresentStatus_t;

/**
	Call Notify Supplementary Services
**/
typedef enum
{
    CALLNOTIFYSS_UNKNOWN,                   ///< UNKNOWN
	CALLNOTIFYSS_CF_ACTIVE,					///< all forwarding SS is active
	CALLNOTIFYSS_CFU_ACTIVE,				///< unconditional call forwarding is active
	CALLNOTIFYSS_CFC_ACTIVE,				///< some of the conditional call forwardings are active
	CALLNOTIFYSS_CFB_ACTIVE,				///< call forwarding on mobile subscriber busy is active
	CALLNOTIFYSS_CFNRY_ACTIVE,				///< call forwarding on no reply is active
	CALLNOTIFYSS_CFNRC_ACTIVE,				///< call forwarding on mobile subscriber not reachable is active
	CALLNOTIFYSS_OUTCALL_FORWARDED,			///< outgoing call forwarded 
	CALLNOTIFYSS_INCALL_FORWARDED,			///< incoming call forwarded 
	CALLNOTIFYSS_FORWARDED_CALL,			///< incoming call is a forwarded call 
	CALLNOTIFYSS_CALL_WAITING,				///< call waiting is active 
	CALLNOTIFYSS_CALLRETRIEVED,				///< retrieve a call 
	CALLNOTIFYSS_CALLONHOLD,				///< call on hold 
	CALLNOTIFYSS_MPTYIND,					///< multiparty call 
	CALLNOTIFYSS_CUGINDEX,					///< closed user group 
	CALLNOTIFYSS_CLIRSUPREJ,				///< connected line identification rejection 
	CALLNOTIFYSS_BAC,						///< Barring of all calls
	CALLNOTIFYSS_OUTCALL_BARRED,			///< Barring of outgoing Call
	CALLNOTIFYSS_BAOC,						///< Barring of all outgoing calls
	CALLNOTIFYSS_BOIC,						///< Barring of outgoing international calls
	CALLNOTIFYSS_BOIC_EX_HC,				///< Barring of outgoing international calls expect those directed to the home PLMN
	CALLNOTIFYSS_INCALL_BARRED,				///< Barring of incoming call
	CALLNOTIFYSS_INCOMING_BARRED,			///< Barrin of all incoming calls
	CALLNOTIFYSS_BAIC_ROAM,					///< Barring of incomming calls when roaming outside home PLMN country
	CALLNOTIFYSS_ECT_INDICATION,			///< ECT call indication
	CALLNOTIFYSS_NAME_INDICATOR,			///< Name Indicator, which is related to CNAP
	CALLNOTIFYSS_CALL_ON_HOLD_RELEASED,		///< Call on hold released
	CALLNOTIFYSS_FORWARD_CHECK_SS_MSG,		///< Forward check SS message
	CALLNOTIFYSS_CALL_DEFLECTED,			///< Call has been deflected (Outgoing)
	CALLNOTIFYSS_DEFLECTED_CALL				///< This is a deflected call (Incomming)
	
} CallNotifySS_t;

/**
	Supplementary Service Codes
**/
typedef enum
{
   SSCODE_ALL_SS       = 0x00,   ///< All SS

   SSCODE_ALL_LI       = 0x10,   ///< All Line Identification
   SSCODE_CLIP         = 0x11,   ///< Calling Line Identification Presentation 
   SSCODE_CLIR         = 0x12,   ///< Calling Line Identification Restriction 
   SSCODE_COLP         = 0x13,   ///< Connected Line Identification Presentation
   SSCODE_COLR         = 0x14,   ///< Connected Line Identification Restriction
   SSCODE_MCI          = 0x15,   ///< Malicious Call Identification 
   SSCODE_CNAP         = 0x19,   ///< Calling NAme presentation

   SSCODE_ALL_FSS      = 0x20,   ///< All Forwarding SS
   SSCODE_CFU          = 0x21,   ///< Call Forwarding Unconditional
   SSCODE_ALL_COND_FSS = 0x28,   ///< All Conditional Forwarding SS
   SSCODE_CFB          = 0x29,   ///< Call Forwarding on Busy
   SSCODE_CFNRY        = 0x2A,   ///< Call Forwarding on Reply
   SSCODE_CFNRC        = 0x2B,   ///< Call Forwarding on not Reachable

   SSCODE_ALL_COSS     = 0x30,   ///< All Call Offering SS includes all forwarding SS
   SSCODE_ECT          = 0x31,   ///< Explicit Call Transfer
   SSCODE_MAH          = 0x32,   ///< Future, Mobile Access Hunting

   SSCODE_All_CCSS     = 0x40,   ///< All Call Completion SS
   SSCODE_CW           = 0x41,   ///< Call Waiting
   SSCODE_CH           = 0x42,   ///< Call Hold
   SSCODE_CCBS         = 0x43,   ///< Future, Completion Call to Busy Subscriber

   SSCODE_All_MPTYSS   = 0x50,   ///< Future, All Multiparty SS
   SSCODE_MPTY         = 0x51,   ///< Multiparty Calls

   SSCODE_ALL_COISS    = 0x60,   ///< Future, All Community Of Interest SS
   SSCODE_CUG          = 0x61,   ///< Closed User Group

   SSCODE_ALL_CSS      = 0x70,   ///< Future, All Charging SS
   SSCODE_AOCI         = 0x71,   ///< Advice Of Charge Information
   SSCODE_AOCC         = 0x72,   ///< Advice Of Charge Charging

   SSCODE_ALL_AITSS    = 0x80,   ///< Future, All Additional Information Transfer SS
   SSCODE_UUS          = 0x81,   ///< Future, User to User Signalling

   SSCODE_BAC          = 0x90,   ///< All Barring SS
   SSCODE_BOC          = 0x91,   ///< Barring of Outgoing Calls
   SSCODE_BAOC         = 0x92,   ///< Barring of All Outgoing Calls
   SSCODE_BOIC         = 0x93,   ///< Barring of Outgoing International Calls
   SSCODE_BOIC_EX_HC   = 0x94,   ///< Barring of Outgoing International Except to Home PLMN
   SSCODE_BIC          = 0x99,   ///< Barring of Incoming Calls
   SSCODE_BAIC         = 0x9A,   ///< Barring of All Incoming Calls
   SSCODE_BAIC_ROAM    = 0x9B,   ///< Barring of All Incoming Calls when Roaming outside home PLMN

   SSCODE_ALL_PLMNSS   = 0xF0,   // not used with Version 1
   SSCODE_PLMN_SS_1    = 0xF1,
   SSCODE_PLMN_SS_2    = 0xF2,
   SSCODE_PLMN_SS_3    = 0xF3,
   SSCODE_PLMN_SS_4    = 0xF4,
   SSCODE_PLMN_SS_5    = 0xF5,
   SSCODE_PLMN_SS_6    = 0xF6,
   SSCODE_PLMN_SS_7    = 0xF7,
   SSCODE_PLMN_SS_8    = 0xF8,
   SSCODE_PLMN_SS_9    = 0xF9,
   SSCODE_PLMN_SS_A    = 0xFA,
   SSCODE_PLMN_SS_B    = 0xFB,
   SSCODE_PLMN_SS_C    = 0xFC,
   SSCODE_PLMN_SS_D    = 0xFD,
   SSCODE_PLMN_SS_E    = 0xFE,
   SSCODE_PLMN_SS_F    = 0xFF

} SSCode_t;								///< SS Code type


/// Optional SS Code
typedef struct
{
	Boolean			is_used;	///< Used Flag
	SSCode_t		value;		///< SS Code 
} OptSSCode_t;


typedef UInt8		SSStatus_t;		///< Supplementary Service Status

/// Optional SS Status
typedef struct
{
	Boolean			is_used;	///< Used Flag	
	SSStatus_t		value;		///< SS Status
} OptSSStatus_t;

/**
	SS Notification Type
**/
typedef enum
{
    SS_NOTIF_UNKNOWN,				///< Unknown SS notification
	SS_NOTIF_OUTCALL_IS_FORWARDED,	///< outgoing call forwarded 
	SS_NOTIF_INCALL_IS_FORWARDED,	///< incoming call forwarded 
	SS_NOTIF_INCALL_IS_A_FORWARDED	///< incoming call is a forwarded call 
} SSNotification_t;


//typedef UInt8		SSNotification_t;	// SS Notification

/// Optional SS Notification
typedef struct
{
	Boolean				is_used;	///< Used Flag
	SSNotification_t	value;		///< SS Notification
} OptSSNotification_t;

/**
	Call Hold indicator
**/
typedef enum
{
	CHINDICTOR_RETRIEVED,
	CHINDICTOR_HELD
} CHIndicator_t;						///< Call Hold Indicator


typedef struct
{
	Boolean			is_used;
	CHIndicator_t	value;
} OptCHIndicator_t;


typedef struct
{
	Boolean			is_used;
	Boolean			value;
} OptNULL_t;

typedef struct
{
	Boolean			is_used;
	UInt16			value;
} OptUInt16_t;


typedef enum 
{
	ECTSTATE_ALERTING,
	ECTSTATE_ACTIVE
} ECTCallState_t;
  
  
typedef struct
{
	TelephoneNumber_t	phone_number;
	Boolean				subaddress_is_used;
	Subaddress_t		subaddress;
} OptECTNum_t;

typedef enum
{
	PRESENT_ALLOWED_ADD,
	PRESENT_RESTRICTED,
	NUMBER_NOT_AVAILABLE,
	PRESENT_RESTRICT_ADD,
	PRESENTATION_NONE
} Presentation_t;

typedef struct
{
	Presentation_t	rdn_choice_type;

	union
	{
 		OptECTNum_t allowed_add;
 		OptECTNum_t restricted_add;
	} rdn_choice;

} ECTRdn_t;

typedef struct
{ 
 	Boolean			is_used;
	ECTCallState_t  ect_call_state;
	ECTRdn_t		rdn_number;		
} OptECTIndicator_t;

typedef struct
{
 	Boolean			is_used;
	UInt8			data_cod_scheme;
	UInt8			length_in_chars;
	USSDString_t	name;
} OptNameSet_t;


typedef struct
{
 	Boolean			is_used;
 	OptNameSet_t 	name_present_allowed;
	OptNULL_t		present_restricted;
 	OptNULL_t		name_unavailable; 
 	OptNameSet_t	name_present_restricted;
} OptCallingName_t;


//This structure is part of the CNAP (Calling NAme Presentation)
/// Callling Name Presentation Structure
typedef struct
{ 
 	Boolean				is_used;
	OptCallingName_t	calling_name;		
} OptNameIndicator_t;   


typedef struct
{
	OptSSCode_t			ss_code;
	OptSSStatus_t		ss_status;
	OptSSNotification_t	ss_notification;
	OptNULL_t			cw_indicator;
	OptCHIndicator_t	ch_indicator;
	OptNULL_t			mpty_indicator;
	OptUInt16_t			cug_index;
	OptNULL_t			clir_suppress_reject;
	OptECTIndicator_t	ect_indicator;
	OptNameIndicator_t	name_indicator;
} FIEIParmNotifyArg_t;


typedef struct
{
	ECTCallState_t		call_state;
	Boolean				present_allowed_add;
	Boolean				present_restricted;
 	Boolean				number_not_available;
	Boolean				present_restricted_add;
 	TelephoneNumber_t	phone_number;
} EctRdnInfo_t;


/// SS call notification parameters
typedef union
{
   UInt16		cug_index;
   EctRdnInfo_t	ect_rdn_info;
   UInt8		callingName[PHASE1_MAX_USSD_STRING_SIZE+1];
} SsNotifyParam_t;

// Notify upper layer of SS call operation
/// SS - higher layer notification of call operation
typedef struct 
{										 
   CallIndex_t		index;          				///< Call identification
   CallNotifySS_t	NotifySS_Oper;  				///< SS operation
   SsNotifyParam_t	notify_param;					///< Notify Parameters
   UInt8			fac_ie[MAX_FACILITY_IE_LENGTH];	///< Raw facilityIE received from the network
} SS_CallNotification_t; 							///< SS Call Notification Type

/**
	Call Action Status
**/
typedef enum
{
	CALLACTIONSTATUS_SUCCEEDED,			///< Call Action succeeded
	CALLACTIONSTATUS_FAILED				///< Call Action failed
} CallActionStatus_t;

/**
	Supplementary Service Action
**/
typedef enum
{
	SSACTION_NONE,						///< No SS Action to do
	SSACTION_ACTIVATION,				///< SS Activation
	SSACTION_DEACTIVATION,				///< SS Deactivation
	SSACTION_INTERROGATION,				///< SS Interrogation
	SSACTION_REGISTRATION,				///< SS Registration
	SSACTION_ERASURE					///< SS Erasure
} SSAction_t;							///< SS Action to do

/**
	MS De-Registration causes
**/
typedef enum
{
	PowerDown	  = 0,					///< MS is powered down
	SIMRemoved	  = 1,					///< SIM was removed
	DetachService = 6,					///< Detach was requested 
	InternalDeactivate = 10				///< Internal deactivation
} DeRegCause_t;							///< Deregistration cause

/**
	AT Command causes for Call release
**/
typedef enum							///< Call Release Causes (see stack's msnu.h) 
{
	ATCAUSE_USER_REL 	= 0,
	ATCAUSE_V24_FAILURE = 1,
	ATCAUSE_TIMER_OUT   = 2	
} CallRelCause_t;

/**
	Line States Values
**/
typedef enum							// Line States (see stack's msnu.h)
{
	LINESTATE_ON	= 0,		///< This represents the status of MS' readiness
	LINESTATE_OFF	= 1			///< for data transfer in a data call. The lines
								///< are DTR, RTS.
} ATDSLineState_t;

/**
	Advice of Charge Status
**/
typedef enum 
{
	AOCSTATUS_DEACTIVE,					///< AOC services are deactive
	AOCSTATUS_ACTIVE,					///< AOC services are active
	AOCSTATUS_SUSPENDED,				///< AOC services are suspended
	AOCSTATUS_ACI_UPDATE				///< AOC ACI has been updated
} AoCStatus_t;  


//
//	NOTE: do not use the values from the Stack
typedef enum {

	MNCAUSE_UNASSIGNED_NUMBER					= 0x01,	///< 1.Number not assigned
	MNCAUSE_NO_ROUTE							= 0x03,	///< 3.No route available 
	MNCAUSE_CHANNEL_UNACCEPTABLE				= 0x06,	///< 6.Unacceptable Channel
	MNCAUSE_OPERATOR_BARRING					= 0x08,	///< 8.Request barred by operator
	MNCAUSE_NORMAL_CALL_CLEARING				= 0x10,	///< 16.Normal call clearing
	MNCAUSE_USER_BUSY							= 0x11,	///< 17.Called party is busy
	MNCAUSE_NO_USER_RESPONDING					= 0x12,	///< 18.User is not responding
	MNCAUSE_USER_ALERTING_NO_ANSWR				= 0x13,	///< 19.No answer to user alerting
	MNCAUSE_MN_CALL_REJECTED					= 0x15,	///< 21.Call rejected
	MNCAUSE_NUMBER_CHANGED						= 0x16,	///< 22.Number has been changed
	MNCAUSE_PRE_EMPTION							= 0x19,	///< 25.Pre-emption
	MNCAUSE_NON_SELECT_USER_CLR					= 0x1A,	///< 26.User Caller Line restriction
	MNCAUSE_DEST_OUT_OF_ORDER					= 0x1B,	///< 27.Destination number/equipment is out of order
	MNCAUSE_INVALID_NUMBER_FORMAT				= 0x1C,	///< 28.Invalid number format
	MNCAUSE_FACILITY_REJECTED					= 0x1D,	///< 29.Facility rejected
	MNCAUSE_RESPONSE_TO_STATUS_ENQ				= 0x1E,	///< 30.Reponse to status enquiry
	MNCAUSE_NORMAL_UNSPECIFIED					= 0x1F,	///< 31.Normal unspecified call release
	MNCAUSE_NO_CIRCUIT_AVAILABLE				= 0x22,	///< 34.No call cicuit available
	MNCAUSE_NETWORK_OUT_OF_ORDER				= 0x26,	///< 38.Network out of order
	MNCAUSE_TEMPORARY_FAILURE					= 0x29,	///< 41.Temporary failure.Try later.
	MNCAUSE_SWITCH_CONGESTION					= 0x2A,	///< 42.Network switch congestion
	MNCAUSE_ACCESS_INFO_DISCARDED				= 0x2B,	///< 43.Access information discarded
	MNCAUSE_REQUESTED_CIRCUIT_NOT_AVAILABLE		= 0x2C,	///< 44.Requested Circuit is not available.
	MNCAUSE_RESOURCES_UNAVAILABLE				= 0x2F,	///< 47.Resources not available
	MNCAUSE_QUALITY_UNAVAILABLE					= 0x31,	///< 49.Requested Quality of service is not available
	MNCAUSE_FACILITY_NOT_SUBSCRIBED				= 0x32,	///< 50.Requested facility is not subscribed
	MNCAUSE_INCOMING_CALLS_BARRED_IN_CUG		= 0x37,	///< 55.Incoming calls are barred in Closed User Group
	MNCAUSE_BEARER_CAPABILITY_NOT_ALLOWED		= 0x39,	///< 57.Data call bearer capability not allowed
	MNCAUSE_BEARER_CAPABILITY_NOT_AVAILABLE		= 0x3A,	///< 58.Requested bearer capability not available.
	MNCAUSE_SERVICE_NOT_AVAILABLE				= 0x3F,	///< 63.Service is not available
	MNCAUSE_BEARER_SERVICE_NOT_IMPLEMENTED		= 0x41,	///< 65.Bearer service is not implemented
	MNCAUSE_ACM_GREATER_OR_EQUAL_TO_ACMMAX		= 0x44,	///< 68.Call Meter value is equal or greater than max value.
	MNCAUSE_FACILITY_NOT_IMPLEMENTED			= 0x45,	///< 69.Facility not implemented.
	MNCAUSE_ONLY_RESTRICTED_DIGITAL				= 0x46,	///< 70.Restricted Digital
	MNCAUSE_SERVICE_NOT_IMPLEMENTED				= 0x4F,	///< 79.Service not implemented.
	MNCAUSE_INVALID_TI							= 0x51,	///< 81.Invalid Transaction Identifier.
	MNCAUSE_USER_NOT_IN_CUG						= 0x57,	///< 87.User is not in Closed User Group.
	MNCAUSE_INCOMPATIBLE_DESTINATION			= 0x58,	///< 88.Destination is incompatible.
	MNCAUSE_INVALID_TRANSIT_NETWORK				= 0x5B,	///< 91.Transit network is invalid
	MNCAUSE_SEMATICS_INCORRECT					= 0x5F,	///< 95.Semantics incorrect.
	MNCAUSE_INVALID_MANATORY_INFORMATION		= 0x60,	///< 96.Invalid mandatory information
	MNCAUSE_MESG_TYPE_NON_EXISTENT				= 0x61,	///< 97.Message type non existent
	MNCAUSE_MESG_TYPE_NOT_COMPATIBLE_WITH_STATE	= 0x62,	///< 98.Received message type non compatible with MS' state
	MNCAUSE_IE_NON_EXISTENT						= 0x63,	///< 99.Information Element does not exist
	MNCAUSE_CONDITIONAL_IE_ERROR				= 0x64,	///< 100.Conditional Info. Element error
	MNCAUSE_MESG_NOT_COMPATIBLE_WITH_STATE		= 0x65,	///< 101.Received message non compatible with MS' state
	MNCAUSE_RECOVERY_ON_TIMER_EXPIRY			= 0x66,	///< 102.Timer expiry recovery
	MNCAUSE_PROTOCOL_ERROR						= 0x6F,	///< 111.Protocol error
	MNCAUSE_INTERWORKING						= 0x7F,	///< 127.Interworking error.
	MNCAUSE_MAXSPECVALUE						= MNCAUSE_INTERWORKING, 
														///< As per 3GPP 24.008 Sec 10.5.4.11
														///< we should not receive any values greater than this from n/w
														///< any values greater are non spec values.

	MNCAUSE_VOID_CAUSE							= 0x100, ///< Should be same as MN_VOID_CAUSE value in stack
	MNCAUSE_OUT_OF_MEMORY						= 0x101,  ///< Should be same as MN_OUT_OF_MEMORY value in stack 
															
//	the following two causes are genereated by Stack, not the Network,
//	so it mapped into a cause for MN for the Stack. In future we need to move these out of 
//  Cause_t enum and have a separate enum for these two. 

	MNCAUSE_RADIO_LINK_FAILURE_APPEARED,		///< Sent when lower layer failure happen before RR is established. Use this cause for call retry 		
	MNCAUSE_REESTABLISHMENT_SUCCESSFUL,
	MNCAUSE_RADIO_LINK_FAILURE_AFTER_RRC		///< Sent when lower layer failure happen after RR is established.
} Cause_t;		///< Cause Values


#define MAX_USER_TO_USER_SIZE	128		///< Max User To User Size


/// Enum : Error Code
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


// Enum : User To User
typedef struct
{
	UInt8		uusProtDisc;					///< User To User Protocol Discreminator
	UInt8		length;							///< Length of the UUS Information
	UInt8		uusInfo[MAX_USER_TO_USER_SIZE];	///< User To User Information
} SS_UserToUser_t;

// Enum : User User Service
typedef enum
{
	SS_USER_USER_SERVICE_1,					///< User User Service 1
	SS_USER_USER_SERVICE_2,					///< User User Service 2
	SS_USER_USER_SERVICE_3					///< User User Service 3
} SS_UusSrvType_t;

// Enum : User User Service
typedef struct
{
	SS_UusSrvType_t	type;					///< Type
	Boolean			required;				///< Required
} SS_UserUserSrv_t;

/**
@code
Structure : User Information
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^			   
			uus										  X		///< User To User
			moredata							  X 		///< More Data
			errorCode						  X				///< Error Code
			cause						  X					///< Cause
@endcode
**/
// Enum : User Information
typedef struct
{
	UInt8				include;				///< Included information
	SS_UserToUser_t		uus;					///< User To User
	Boolean				moreDate;				///< More Data
	UInt8				uuSrvCount;				///< User User Service Counter
	SS_UserUserSrv_t	uuSrv[3];				///< User User Service
	SS_ErrorCode_t		errorCode;				///< Error Codde
	Cause_t				cause;					///< Cause
	UInt8				returnResult;			///< ReturnResult (Bit:0 UUS1, Bit:1 UUS2, Bit:2 UUS3)
	UInt8				callIndex;				///< Call Index
} SS_UserInfo_t;

// Call release info
typedef struct
{
    UInt8 callIndex;
    Cause_t cause;
} CallReleaseInfo_t;

/**
	Voice Call Parameters
	NOTE: Any client that is using this structure in an API call should initialize
	the structure to 0 before assigning any values. 
	Any unused parameter should be defaulted to 0.
*/	
typedef struct{

	Boolean			isFdnChkSkipped;					///< TRUE if FDN check by platform in CC_MakeVoiceCall() is to be skipped (e.g. for STK Setup Call)
	CLIRMode_t		clir;								///< Caller Line ID Restriction
	CUGInfo_t		cug_info;							///< Closed User Group Info.
	Boolean			auxiliarySpeech;					///< Auxillary Speech line
	Boolean			isEmergency;						///< Is this an emergency call
	Subaddress_t	subAddr;							///< Sub Address Type
	CC_BearerCap_t	bearerCap;							///< Bearer Capability to be used for the call
	UInt8			dtmfLength;							///< DTMF length
	DTMFTone_t		dtmfTones[DTMF_TONE_ARRAY_SIZE];	///< DTMF Tone
	SS_UserInfo_t	uusInfo;							///< User To User Information;
} VoiceCallParam_t;



typedef enum {

	GSMCAUSE_SUCCESS,								///< Request Success
	GSMCAUSE_REG_INVALID_SIM,						///< SIM card is not valid
	GSMCAUSE_REG_INVALID_ME,						///< Invlaid Mobile Equipment
	GSMCAUSE_REG_NO_NETWORK,						///< couldn't find any network
	GSMCAUSE_REG_NO_GPRS,							///< GSM-only cell, no GPRS network
	GSMCAUSE_REG_NO_ACCESS,							///< No access to register
	GSMCAUSE_REG_SERVICE_DISABLED,					///< Registration service is disabled
	GSMCAUSE_REG_NO_CAUSE,							///< No Cause value reported
	GSMCAUSE_ERROR_TIMEOUT,							///< Timed out waiting for the request
	GSMCAUSE_ERROR_UNKNOWN_SUBSCRIBER,				///< Subscriber of this request is unknown
	GSMCAUSE_ERROR_ILLEGAL_SUBSCRIBER,				///< Subscriber illegal
	GSMCAUSE_ERROR_BEARER_SVC_NOT_PROVISIONED,		///< Bearer Service not provisioned
	GSMCAUSE_ERROR_TELE_SVC_NOT_PROVISIONED,		///< Tele Service not provisioned 
	GSMCAUSE_ERROR_ILLEGAL_EQUIPMENT,				///< Equipment is not legal
	GSMCAUSE_ERROR_CALL_BARRED,						///< Call is barred
	GSMCAUSE_ERROR_ILLEGAL_SS_OPERATION,			///< Illegal Supplementary Service operation
	GSMCAUSE_ERROR_SS_ERROR_STATUS,					///< Supplementary Service Error
	GSMCAUSE_ERROR_SS_NOT_AVAILABLE,				///< This SS service is unavailable
	GSMCAUSE_ERROR_SS_SUBSCRIPTION_VIOLATION,		///< The request is in violation of the services subscribed
	GSMCAUSE_ERROR_SS_INCOMPATIBILITY,				///< Service incompatible, request failed.
	GSMCAUSE_ERROR_FACILITY_NOT_SUPPORT,			///< Requested facility is not supported
	GSMCAUSE_ERROR_ABSENT_SUBSCRIBER,				///< Subscriber is absent
	GSMCAUSE_ERROR_SYSTEM_FAILURE,					///< System failure
	GSMCAUSE_ERROR_DATA_MISSING,					///< Data is incomplete in the request
	GSMCAUSE_ERROR_UNEXPECT_DATA_VALUE,				///< Unexpected data in the request. Check request and try again.
	GSMCAUSE_ERROR_PASSWD_REG_FAILURE,				///< This SS service is unavailable
	GSMCAUSE_ERROR_NEGATIVE_PASSWD_CHECK,			///< Password check negative
	GSMCAUSE_ERROR_MAX_PASSWD_ATTEMPTS_VOILATION,	///< Exceeded allowed number of password check attempts.
	GSMCAUSE_ERROR_UNKNOWN_ALPHABET,				///< Unknown alphabet in request
	GSMCAUSE_ERROR_USSD_BUSY,						///< Unstructured SS Data busy.
	GSMCAUSE_ERROR_MAX_MPTY_EXCEEDED,				///< Maximum number of multiparty calls exceeded
	GSMCAUSE_ERROR_RESOURCES_NOT_AVAIL,				///< Necessary resources unavailable by the network
	GSMCAUSE_ERROR_REJECTED							///< Request rejected
} NetworkCause_t;



/// Lower layer compabilitibilty - part of BC of a data call
typedef struct
{
	UInt8			val[18];  ///< Release 99 LLC could be upto 18 bytes.
} LLCompatibility_t;

/// Higher layer compabilitibilty - part of BC of a data call
typedef struct
{
	UInt8			val[5];
} HLCompatibility_t;

/**
	Call Service Type
**/
typedef enum {
	CALLSERVICETYPE_UNKNOWN			 = 0, ///< GSM stack const MNDS_SERVICE_TYPE_UNKNOWN,		
										  ///< for most MT Data Calls
	CALLSERVICETYPE_SPEECH_RECORDING = 1, ///< GSM stack const MNDS_SERVICE_TYPE_SPEECH,		
										  ///< for Stack Speech Recording (future feature)
	CALLSERVICETYPE_ALTER2NORMAL	 = 2, ///< GSM stack const MNDS_SERVICE_TYPE_ALTER2NORMAL,	
										  ///< for Speech/Fax, not supported by MTI
	CALLSERVICETYPE_SPEECH			 = (CALLSERVICETYPE_ALTER2NORMAL+1)
										  ///< GSM stack const MNDS_SERVICE_TYPE_ALTER2NORMAL+1	
										  ///< used by MTI only
} CallServiceType_t;


// represents MS Channel Type, values assigned by Stack
/**
	Channel Type
**/
typedef enum
{
	MSCHANNELTYPE_FULL_RATE_ONLY = FULL_RATE_ONLY,///< Full rate only
	MSCHANNELTYPE_DUAL_RATE		 = DUAL_RATE,	///< Half and Full rate
	MSCHANNELTYPE_SDCCH_ONLY	 = SDCCH_ONLY	///< Signalling only
} MSChannelType_t;

// Location Area and Cell Site Information
typedef UInt16 LACode_t;		///< Location Area Code

/// PLMN ID
typedef struct
{
	UInt16 mcc;					///< unconverted Mobile Country Code
	UInt8 mnc;					///< unconverted Mobile Network Code
	Boolean is_forbidden;		///< TRUE, if forbidden
} PLMNId_t;

/**
Enum:	The type of plmn
		Should be in sync with T_NETWORK_TYPE provided by the stack
**/
typedef enum {
	
	MSNWTYPE_HOME_PLMN,				///< The PLMN is a Home PLMN of the MS
	MSNWTYPE_PREFERRED_PLMN,		///< The PLMNID is present in the Preferred PLMN list in SIM
	MSNWTYPE_FORBIDDEN_PLMN,		///< The PLMNID is in the forbidden PLMN list stored in the MS
	MSNWTYPE_OTHER_PLMN,			///< Other PLMN
	MSNWTYPE_INVALID	= 255		///< Used when the stack doesn't provide the network type
} MSNwType_t;						///< MS Network Type

/**
	This enum is used to indicate whether the PLMN is stored in the EONS, NITZ
	or LOOKUPTABLE. This information is filled in by CAPI and used by upper layer
**/
typedef enum
{
	PLMN_NAME_TYPE_INVALID,
	PLMN_NAME_TYPE_EONS,		///< retreived from SIM EONS
	PLMN_NAME_TYPE_CPHS,		///< retrieved from SIM CPHS
	PLMN_NAME_TYPE_NITZ,		///< retrieved from NITZ
	PLMN_NAME_TYPE_LKUP_TABLE	///< retrieved from LookupTable
} PLMNNameType_t;

/// PLMN Name Structure
typedef struct
{
	ALPHA_CODING_t 	coding;			///< GSM Alphabet, UCS2_80, UCS2_81 or UCS2_82 
	UInt8			name_size;		///< Number of bytes used in "name" 
	UInt8			name[60];		///< For ASCII encoding, the data is not guaranteed to be NULL terminated. 
									///< For UCS2 encoding, the data begins with the byte after the UCS2 encoding byte, 
									///< i.e. After 0x80, 0x81 or 0x82.
	PLMNNameType_t	nameType;		///< Indicates whether the PLMN is retrieved from NITZ, EONS, lookuptable.

	Boolean			is_pnn_1st_rec;	///< TRUE if name is obtained from first record in EF-PNN in SIM. This element is applicable only if "nameType == PLMN_NAME_TYPE_EONS".
									///< Section 4.2.58 of 3GPP 31.102 says if a RPLMN's name is obtained from the first record in EF-PNN it is considered HPLMN.
} PLMN_NAME_t;

/// MS PLMN Name
typedef struct {
	Boolean			matchResult;	///< Match result
	PLMN_NAME_t		longName;		///< Long plmn name
	PLMN_NAME_t		shortName;		///< Short plmn name
}MsPlmnName_t;

/// Structure: PLMN returned in a PLMN search 
typedef struct
{
	UInt16				mcc;			///< unconverted Mobile Country Code - 3 digits
	UInt16				lac;			///< unconverted, store the LAC of the highest power
	UInt8				mnc;			///< unconverted Mobile Network Code 2-3 digits
	Boolean				is_forbidden;	///< TRUE, if forbidden
	UInt8				rat;			///< RAT_NOT_AVAILABLE(0),RAT_GSM(1),RAT_UMTS(2)
	MSNwType_t			network_type;	///< Type of PLMN , HPLMN, FORBIDDEN,PREFERRED as defined in MSNwType_t
	MsPlmnName_t		ucs2Name;		///< Name of the PLMN ,in ucs2 character format. 
	MsPlmnName_t		nonUcs2Name;	///< Name of the PLMN in non-UCS2 character format
} SEARCHED_PLMNId_t;				///< PLMN List sent in the PLMN_LIST_CNF message to the upper layer.

#define EQUIV_PLMN_LIST_SIZE 16		///< Equivalment PLMN List

/// Equivalent PLMN List Structure
typedef struct
{
	UInt8	length;		///< Number of PLMN in "equiv_plmn" 
	T_PLMN	equiv_plmn[EQUIV_PLMN_LIST_SIZE];
} EQUIV_PLMN_LIST_t;
	
/**
	Cause Values returned in PLMN Search
**/

typedef enum {
	PLMNCAUSE_SUCCESS,				///< Success, found networks
	PLMNCAUSE_NETWORK_NOT_FOUND,	///< Networks not found
	PLMNCAUSE_OPERATION_NOT_ALLOWED,///< Operation not allowed
	PLMNCAUSE_USER_ABORT,			///< Operation aborted by user
	PLMNCAUSE_ACTIVE_CALL,			///< when any call is active plmn search is rejected
} PlmnCause_t;

/// List returned after PLMN search
typedef struct
{
	PlmnCause_t			plmn_cause;
	UInt8				num_of_plmn;	///< Number of PLMN in "searched_plmn" list 
	SEARCHED_PLMNId_t 	searched_plmn[MAX_PLMN_SEARCH];
} SEARCHED_PLMN_LIST_t;

/// Location Area Information
typedef struct
{
	PLMNId_t plmn;				///< PLMN value, MCC and MNC
	LACode_t lac;				///< Location Area Code, unconverted
} LAInfo_t;

typedef UInt16 CellInfo_t;		///< Cell Site Information, unconverted

/**
@code
*         8         7         6          5        4         3       2       1     Bit
*       Rsvd       rsvd       rsvd      HR        FR        GSM     GSM     GSM
* 				     					AMR      AMR       	EFR     HR      FR
*       0           0          0         x        x          x       x      x      Support
*
*		Where support bit states:
*		0 = codec not supported
*		1 = codec supported
@endcode
**/
typedef UInt8 MS_SpeechCodec_t;
#define CODEC_GSMFR	0x01
#define CODEC_GSMHR	0x02
#define CODEC_EFR	0x04
#define CODEC_AMRFR	0x08
#define	CODEC_AMRHR	0x10
#define CODEC_AMRWB 0x20

// valid combinations of the codecs 
// As per 3GPP 24.008 10.5.4.5 we need to 
#define MSCHANNELMODE_GSMHR_GSMFR					(MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_HALF)
#define MSCHANNELMODE_GSMFR_EFR					(MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V2)
#define MSCHANNELMODE_GSMFR_AMRFR					(MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V3)
#define MSCHANNELMODE_AMRHR_GSMFR					(MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_FULL)
#define MSCHANNELMODE_GSMHR_GSMFR_EFR				(MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_HALF | MSCHANNELMODE_SPEECH_FULL_V2)
#define MSCHANNELMODE_GSMHR_GSMFR_AMRFR 			(MSCHANNELMODE_SPEECH_HALF | MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V3)
#define MSCHANNELMODE_GSMHR_AMRHR_GSMFR 			(MSCHANNELMODE_SPEECH_HALF | MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_FULL)
#define MSCHANNELMODE_GSMFR_EFR_AMRFR				(MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V2 | MSCHANNELMODE_SPEECH_FULL_V3)
#define MSCHANNELMODE_AMRHR_GSMFR_EFR				(MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_FULL_V2)
#define MSCHANNELMODE_AMRHR_GSMFR_AMRFR			(MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V3)
#define MSCHANNELMODE_GSMHR_GSMFR_EFR_AMRFR 		(MSCHANNELMODE_SPEECH_HALF | MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V2 | MSCHANNELMODE_SPEECH_FULL_V3)
#define MSCHANNELMODE_AMRHR_GSMFR_EFR_AMRFR		(MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V2 | MSCHANNELMODE_SPEECH_FULL_V3)
#define MSCHANNELMODE_GSMHR_AMRHR_GSMFR_EFR		(MSCHANNELMODE_SPEECH_HALF | MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V2)
#define MSCHANNELMODE_GSMHR_AMRHR_GSMFR_AMRFR 		(MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_HALF |MSCHANNELMODE_SPEECH_FULL_V3 | MSCHANNELMODE_SPEECH_FULL )
#define MSCHANNELMODE_GSMHR_AMRHR_GSMFR_EFR_AMRFR	(MSCHANNELMODE_SPEECH_FULL_V3 | MSCHANNELMODE_SPEECH_FULL_V2 | MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_HALF)



// represents MS Channel Mode, values assigned by sysparm
typedef UInt16	MSChannelMode_t;
#define MSCHANNELMODE_SIGNALLING_ONLY	SIGNALLING_ONLY		///< Signalling only Channel
#define MSCHANNELMODE_SPEECH_FULL		SPEECH_FULL			///< Full rate Speech Cahnnel
#define MSCHANNELMODE_SPEECH_HALF		SPEECH_HALF			///< Half rate Speech Cahnnel
#define MSCHANNELMODE_DATA_144			DATA_144			///< 14400bps Data Channel		
#define MSCHANNELMODE_DATA_96			DATA_96				///< 9600bps Data Channel		
#define MSCHANNELMODE_DATA_48_FULL		DATA_48_FULL		///< 4800bps Full rate Data Channel		
#define MSCHANNELMODE_DATA_48_HALF		DATA_48_HALF		///< 4800bps Half rate Data Channel		
#define MSCHANNELMODE_DATA_24_FULL		DATA_24_FULL		///< 2400bps Full rate Data Channel		
#define MSCHANNELMODE_DATA_24_HALF		DATA_24_HALF		///< 2400bps Half rate Data Channel		
#define MSCHANNELMODE_SPEECH_FULL_V2	SPEECH_FULL_V2		///< Full rate Version 2 Speech Cahnnel
#define MSCHANNELMODE_SPEECH_FULL_V3	SPEECH_FULL_V3		///< Full rate Version 3 Speech Cahnnel
#define MSCHANNELMODE_SPEECH_HALF_V2	SPEECH_HALF_V2		///< Half rate Version 2 Speech Cahnnel
#define MSCHANNELMODE_SPEECH_HALF_V3	SPEECH_HALF_V3		///< Half rate Version 3 Speech Cahnnel

//new definitions for channel mode due to AMR - WB
#define MSCHANNELMODE_SPEECH_FULL_V4	SPEECH_FULL_V4		///<Full rate version 4 OFR-AMR WB
#define MSCHANNELMODE_SPEECH_FULL_V5	SPEECH_FULL_V5		///<Full rate version 5 FB-AMR WB 
#define MSCHANNELMODE_SPEECH_HALF_V4	SPEECH_HALF_V4		///<Half rate version 4 OHR-AMR WB 
#define MSCHANNELMODE_SPEECH_HALF_V6	SPEECH_HALF_V6		///<Half rate version 6 OHR-AMR
#define MSCHANNELMODE_SPEECH_FULL_V6	SPEECH_FULL_V6		///<Full rate Version 6 UMTS-AMR WB 
															///need to confirm, this mapping is not mentioned in 3GPP 48.008)
#define	IS_MSCHANNELMODE_SIGNALLING_ONLY( v )	\
	( v & MSCHANNELMODE_SIGNALLING_ONLY )

#define	IS_MSCHANNELMODE_SPEECH( v )	\
	( v & (MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_HALF | \
	MSCHANNELMODE_SPEECH_FULL_V2 | MSCHANNELMODE_SPEECH_FULL_V3 | \
	MSCHANNELMODE_SPEECH_HALF_V2 | MSCHANNELMODE_SPEECH_HALF_V3 | \
	MSCHANNELMODE_SPEECH_FULL_V4| MSCHANNELMODE_SPEECH_FULL_V5 | \
	MSCHANNELMODE_SPEECH_HALF_V4 | MSCHANNELMODE_SPEECH_HALF_V6 | MSCHANNELMODE_SPEECH_FULL_V6 ) )	
	
#define	IS_MSCHANNELMODE_DATA( v )		\
	( v & (MSCHANNELMODE_DATA_96 | 		\
	MSCHANNELMODE_DATA_48_FULL | MSCHANNELMODE_DATA_48_HALF | \
	MSCHANNELMODE_DATA_24_FULL | MSCHANNELMODE_DATA_24_HALF | \
	MSCHANNELMODE_DATA_144 ))


#define TBSParameter_t		void				// To Be Specified Parameter

typedef SDL_PId				PId_t;				///< Process ID, used by SDL


#define BEARER_CAPABILITY_IEI	0x04		///< Bearer Capability Information Element Identifier

// Subaddresses Information Element Identifiers
#define CALLING_PARTY_SUBADD_IEI	0x5D	///< Calling Party Subaddress Information Element Identifier
#define CALLED_PARTY_SUBADD_IEI		0x6D	///< Called Party Subaddress Information Element Identifier
#define CONNECTED_SUBADDR_IEI		0x4D	///< Connected Party Subaddress Information Element Identifier

typedef UInt8	*NetworkName_t;					// See GSM 04.08 for Structure

// See GSM 09.02 sections 17.7.9 and 17.7.10

#define allBearerServices			0x00
#define allDataCDA					0x10
#define allDataCDS					0x18
#define allPADAccessCA				0x20
#define allPADAccessPDS				0x28
#define allDataPDS					0x28
#define allCircuitAsync				0x50
#define allCircuitSync				0x58
#define allAsync					0x60
#define allSync						0x68
#define allAltSpeechCDA				0x30
#define allAltSpeechCDS				0x38
#define allSpeechFollowedByCDA		0x40
#define allSpeechFollowedByCDS		0x48

#define allVoiceGroupCall			0x91
#define	allVoiceBroadcastCall		0x92

#define allTeleServices				0x00
#define allSpeech					0x10
#define allTelephony				0x11
#define allEmergencyCalls			0x12
#define allSMS						0x20
#define allFAX						0x60
#define allDataTeleservices			0x70
#define	allTeleServicesNoSMS		0x80

/* Teleservice code for ALS (Alternative Line Service) added to Section 17.7.9 of GSM 09.02, see Appendix 1 of CPHS spec */ 
#define allAuxiliarySpeech			0xD0  
#define allAuxiliaryTelephony		0xD1  

/**
	MS power on cause
**/
#ifdef WDT_POWER_CAUSE
typedef enum {
	POWER_ON_CAUSE_NONE = 0,

	POWER_ON_CAUSE_NORMAL = 2,			///< normal power up by power-on-key
	POWER_ON_CAUSE_ALARM = 4,			///< power up due to PMU alarm expiry
	POWER_ON_CAUSE_CHARGING_OFF = 6,	///< power up due to charger plugged in while phone in off state
	POWER_ON_CAUSE_CHARGING_ON = 8,		///< power up due to charger plugged in with power-on-key pressed
	POWER_ON_CAUSE_ERR_RESET = 10,		///< soft reset due to software assertion
	POWER_ON_CAUSE_STK_RESET = 12,		///< soft reset due to SIM reset
	POWER_ON_CAUSE_WATCHDOG_RESET = 14,	///< soft reset due to watchdog timer expiry
	POWER_ON_CAUSE_USB_DL = 16,			///< soft reset due to USB flash download
	POWER_ON_CAUSE_SMPL = 18,			///< power up due to SMPL condition

	POWER_ON_CAUSE_INVALID = 126			///< Invalid
} PowerOnCause_t;
#else
typedef enum {
	POWER_ON_CAUSE_NONE = 0,

	POWER_ON_CAUSE_NORMAL,			///< normal power up by power-on-key
	POWER_ON_CAUSE_ALARM,			///< power up due to PMU alarm expiry
	POWER_ON_CAUSE_CHARGING_OFF,	///< power up due to charger plugged in while phone in off state
	POWER_ON_CAUSE_CHARGING_ON,		///< power up due to charger plugged in with power-on-key pressed
	POWER_ON_CAUSE_ERR_RESET,		///< soft reset due to software assertion
	POWER_ON_CAUSE_STK_RESET,		///< soft reset due to SIM reset
	POWER_ON_CAUSE_WATCHDOG_RESET,	///< soft reset due to watchdog timer expiry
	POWER_ON_CAUSE_USB_DL,			///< soft reset due to USB flash download
	POWER_ON_CAUSE_SMPL,			///< power up due to SMPL condition

	POWER_ON_CAUSE_INVALID			///< Invalid
} PowerOnCause_t;
#endif


/**
	This enum is used to identify API Client command indentifier.
**/
typedef enum
{
	CC_CALL_ACCEPT,	///< Used to identify the call to CC_Voice/Video/DataCallAccept() 
	CC_CALL_REJECT	///< Used to identify the call to CC_EndCall()
} ClientCmd_t;

/**
	This enum is used to indicate GPRS suspend cause
**/
typedef enum
{
	SUSPEND_NO_CAUSE = 0,
	SUSPEND_LOCATION_UPDATING = 1,
	SUSPEND_MOBILE_ORIG_CALL_EST = 2,
	SUSPEND_MOBILE_TERM_CALL_EST = 3,
	SUSPEND_ROUTING_AREA_UPDATING = 4,
	SUSPEND_NO_COVERAGE = 5,
	SUSPEND_OTHER_CAUSE = 6
} SuspendCause_t;

/**
	This enum is used to indicate radio activity --- direction
**/
typedef enum
{
	RADIO_UPLINK = 0,
	RADIO_DOWNLINK = 1,
	RADIO_BIDIRECTION = 2
} RadioDirection_t;

/**
	This enum is used to indicate radio activity --- status (on/off)
**/
typedef enum
{
	RADIO_ON = 0,
	RADIO_OFF = 1
} RadioStatus_t;


/**
	This enum is used to indicate ms 2G radio establish state
**/
typedef enum
{
	MS_IDLE_STATE = 0,
	MS_SIGNAL_STATE = 1,
	MS_CALL_STATE = 2,	///< RR Established
} MSStatus_t;

// Define DTX_FROM_BS values
typedef enum {
	DTX_MAY = 0,
	DTX_USE,
	DTX_NOT
} MS_DTXFromBs_t;	

// Define the Cell Barr Change Status
typedef enum {
	CELL_BARR_ACCEPTED = 0, 	/* default */
	CELL_BARR_REVERSED,
	CELL_BARR_DISCARD
} CellBarrChangeStatus_t;

// Define the cell priority
typedef enum {
	CELL_PRIORITY_NORM = 0, 	/* default */
	CELL_PRIORITY_BARRED,
	CELL_PRIORITY_LOW
} CellPriorityStatus_t;


// GPRS Ciphering Algorithm 
typedef enum {
	GPRS_CIPH_NO_USED	= 0,		/* default */
	GPRS_CIPH_GEA1		= 1,
	GPRS_CIPH_GEA2		= 2,
	GPRS_CIPH_GEA3		= 3,
	GPRS_CIPH_GEA4		= 4,
	GPRS_CIPH_GEA5		= 5,
	GPRS_CIPH_GEA6		= 6,
	GPRS_CIPH_GEA7		= 7
} GPRS_CiphAlg_t;


// Define 2G HO engineering mode
typedef enum {
	GSM_HO_INTER_CELL	= 0,			
	GSM_HO_INTRA_CELL,	
	GSM_HO_MAX_TYPES,		
	GSM_HO_INVALID					/* default */
} GSMHOFlag_t;


typedef enum {
	GSM_HO_GSM_TO_GSM	= 0,
	GSM_HO_GSM_TO_DCS,	
	GSM_HO_DCS_TO_GSM,	
	GSM_HO_DCS_TO_DCS,
	GSM_HO_MAX_BAND_SWITCH,
	GSM_HO_INVALID_BAND_SW 
} GSMHOBandSwitch_t;


typedef enum {
	CHANNEL_TYPE_THR0		= 0,
	CHANNEL_TYPE_THR1,
	CHANNEL_TYPE_AHS0,
	CHANNEL_TYPE_THS1,
	CHANNEL_TYPE_TFR,
	CHANNEL_TYPE_AFS,
	CHANNEL_TYPE_TEFR,
	CHANNEL_TYPE_F144,
	CHANNEL_TYPE_F96,
	CHANNEL_TYPE_F72,
	CHANNEL_TYPE_F48,
	CHANNEL_TYPE_F24,
	CHANNEL_TYPE_H480,
	CHANNEL_TYPE_H481,
	CHANNEL_TYPE_H240,
	CHANNEL_TYPE_H241,
	CHANNEL_TYPE_FA,
	CHANNEL_TYPE_FAH0,
	CHANNEL_TYPE_FAH1,
	CHANNEL_TYPE_PCCC,
	CHANNEL_TYPE_PBCC,
	CHANNEL_TYPE_PAGC,
	CHANNEL_TYPE_PNDR,
	CHANNEL_TYPE_PDTC,
	CHANNEL_TYPE_NDRX,
	CHANNEL_TYPE_SDCC,
	CHANNEL_TYPE_AGCH,
	CHANNEL_TYPE_CCCH,
	CHANNEL_TYPE_CBCH,
	CHANNEL_TYPE_BCCH,
	CHANNEL_TYPE_SEAR,
	CHANNEL_TYPE_NSPS

} GSMChannelType_t;

// Service state
typedef enum
{
    UNKNOWNSTATE,
	NO_NETWORK_AVAILABLE,          ///<No Network avialable
    SEARCH_FOR_NETWORK,            ///<Mobile is searching for network to camp.Services are not yet available.
    EMERGENCY_CALLS_ONLY,          ///<Only Emergency calls are allowed. GPRS is available but not attached.
    LIMITEDSERVICE,               ///<Mobile is in limited service - only Emergency calls are allowed. GPRS is available but not attached.
    FULL_SERVICE,                  ///<Mobile is in normal service
    PLMN_LIST_AVAILABLE,           ///<PLMN List 
    DISABLEDSTATE,                      ///<Disabled
    DETACHEDSTATE,                        ///<Detached
    NOGPRSCELL,                  ///<MS not being camped on a cell or registered
    SUSPENDEDSTATE,                     ///<Suspended
} Service_State_t;
/**
	This structure is used for all the GAS related nvram classmark parameters
	that needs to be set in the stack at poweron.
	All the elements should be in sync with T_GAS_CONFIG_PARAMS defnd in msnu.h
**/
typedef struct 
{

    Boolean	auto_pwr_updated;
    Boolean auto_pwr_red;
    Boolean egprs_updated;
    Boolean egprs_support;
    Boolean egprs_msc_updated;
    UInt8 	egprs_msc;
    Boolean gprs_msc_updated;
    UInt8 	gprs_msc;
    Boolean rep_acch_updated;
    UInt8 	rep_acch;
    Boolean power_class_8psk_updated;
    UInt8 	power_class_8psk;
    Boolean power_cap_8psk_updated;
    UInt8 	power_cap_8psk;
    Boolean geran_fp1_updated;
    Boolean geran_fp1;
    Boolean u3g_ccn_updated;
    Boolean u3g_ccn_support;
    Boolean nondrx_timer_updated;
    UInt8 	nondrx_timer;
    Boolean split_pg_cycle_updated;
    UInt8 	split_pg_cycle;
    Boolean eda_support_updated;
    Boolean eda_support;
    Boolean darp_support_updated;
    Boolean darp_support;
}
GASConfigParam_t;

/**
	This structure is used for all the UAS related nvram classmark parameters
	that needs to be set in the stack at poweron.
	All the elements should be in sync with T_UAS_CONFIG_PARAMS defnd in msnu.h
**/
typedef struct
{
    Boolean integrity_updated;
    Boolean integrity_support;
    Boolean ciphering_updated;
    Boolean ciphering_support;
    Boolean urrc_version_updated;
    UInt8 	urrc_version;
    Boolean fake_security_updated;
    Boolean fake_security;
	Boolean interrat_nacc_updated;
    Boolean interrat_nacc_support;
}
UASConfigParam_t;

/**
	This structure is used for all the nas related nvram classmark parameters
	that needs to be set in the stack at poweron.
	All the elements should be in sync with T_NAS_CONFIG_PARAMS defnd in msnu.h
**/
typedef struct
{
    Boolean a5_alg_updated;
    UInt8	a5_alg_supp;
    Boolean gea_alg_updated;
    UInt8 	gea_alg_supp;
    Boolean qos_ext_updated;
    UInt8 qos_ext;
    Boolean acq_order_updated;
    UInt8 	rat_acq_order;
/* MobC00061383/CSP204798 */
	Boolean non_3gpp_hplmn_timer_updated ;    
    Boolean non_3gpp_hplmn_timer_supported ; // Indicates if UE supports the non-3gpp HPLMN search timer
/* MobC00061383/CSP204798 */

}
NASConfigParam_t;

/**
	This structure is used for all the nvram classmark parameters
	that needs to be set in the stack at poweron.
	All the elements should be in sync with T_NVRAM_CLASSMARK defnd in msnu.h
**/

typedef struct 
{
	GASConfigParam_t gasConfigParams;
    UASConfigParam_t uasConfigParams;
    NASConfigParam_t nasConfigParams;
	
}NVRAMClassmark_t;

/**
	This structure is used to pass the call setup parameters internally in CAPI. Callcontrol uses this structure
	to send voice call request to SIM that in turn queries the SIM if call control by SIM is enabled and then passes the 
	same structure to MNCC to process the request.
**/
typedef struct
{
	PartyAddress_t			partyAdd;
	Boolean					isEmergency;
	Boolean					auxiliarySpeech;
	Subaddress_t			subAddr;
	CLIRMode_t				clir_mode;
	CUGInfo_t				cug_info;
	CC_BearerCap_t		 	bearerCap;
	UInt8					dtmfLength;							///< DTMF length
	DTMFTone_t				dtmfTones[DTMF_TONE_ARRAY_SIZE];	///< DTMF Tone
	SS_UserInfo_t			uusInfo;							///< User To User Information;
} CCParmSend_t;

typedef struct
{
	UInt32                  mask;                               ///< mask
	Boolean					in_cell_dch_state;                  ///< True if UE in DCH cell state
	Boolean					hsdpa_ch_allocated;                 ///< True if HSDPA Channel allocated
}Uas_Conn_Info;

typedef struct
{
	Uas_Conn_Info in_uas_conn_info;		
} MSUe3gStatusInd_t;

typedef struct 
{
	PLMNId_t				plmnId;             ///< PLMN information
	UInt16					cellId;             ///< Cell Id
	UInt16					lac;                ///< LAC
	UInt8					rac;                ///< RAC
	UInt16					rncId;				///< RNC id valid for 3G cell only
} MSCellInfoInd_t;

#endif


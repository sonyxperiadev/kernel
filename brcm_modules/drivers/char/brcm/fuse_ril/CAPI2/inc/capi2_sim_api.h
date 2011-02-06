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
*   @file   capi2_sim_api.h
*
*   @brief  This file defines the interface for CAPI2 SIM API.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_SIMAPIGroup   SIM API
*   @ingroup    CAPI2_SIMGroup
*
*   @brief      This group defines the interfaces to the SIM system and provides
*				API documentation. Using the following functions, the user can
*				read/update all the EFiles as defined in GSM 11.11 and the
*				Common PCN Handset Specification (CPHS). 
****************************************************************************/


#ifndef _CAPI2_SIM_API_H_
#define _CAPI2_SIM_API_H_

#ifdef __cplusplus
extern "C" {
#endif
   
#include "capi2_msnu.h"
#include "capi2_mstypes.h"
#include "capi2_resultcode.h"
#include "capi2_taskmsgs.h"

//-------------------------------------------------
// Data Structure
//-------------------------------------------------
#define SMSPARAM_SCA_SZ			12

#define ICC_DIGITS						20	///< Max. ICC identification digits

#define UNKNOWN_PLMN_INDEX 0xFFFF ///< Indicate all PLMN entries are requested

#define MAX_NUM_OF_PLMN 150 ///< Maximum of PLMN entries in SIM file

#define USIM_BASIC_CHANNEL_SOCKET_ID 0	///< Socket ID for basic channel reserved for USIMAP application

#define	CAPI2_SST_SIZE                  15  ///< Up to 15 bytes defined in 3G spec TS 151011
#define CAPI2_CPHS_INFO_SIZE            3   ///< Len of CPHS Info
#define CAPI2_MAX_CPHS_ONS_LEN          30  ///< Maximum size of CPHS ONS string
#define CAPI2_MAX_CPHS_ONSS_LEN         10  ///< Maximum size of CPHS ONSS string
#define CAPI2_ECC_REC_LIST_SIZE         18 ///< Ecc Record Table size

/**
SIM Access Types
**/
typedef enum
{
	SIMACCESS_SUCCESS,					///< Access/Verify was successful
	SIMACCESS_INCORRECT_CHV,			///< Verify failed, at least one attempt left
	SIMACCESS_BLOCKED_CHV,				///< Verify failed, CHV is blocked
	SIMACCESS_NEED_CHV1,				///< No access, need CHV1
	SIMACCESS_NEED_CHV2,				///< No access, need CHV2
	SIMACCESS_NOT_NEED_CHV1,			///< CHV1 available and must not be entered
	SIMACCESS_NOT_NEED_CHV2,			///< CHV2 available and must not be entered
	SIMACCESS_NO_ACCESS,				///< No access, not allowed (NVR or ADM)
	SIMACCESS_INVALID,					///< No access, unusable when invalidated file
	SIMACCESS_MEMORY_ERR,				///< Access failed, memory problem
	SIMACCESS_OUT_OF_RANGE,				///< Access failed, invalid address
	SIMACCESS_NOT_FOUND,				///< Access failed, no such file / pattern not found
	SIMACCESS_MAX_VALUE,				///< Increase failed, already at max val
	SIMACCESS_INCORRECT_PUK,			///< Unblock failed, incorrect PUK
	SIMACCESS_BLOCKED_PUK,				///< Unblock failed, PUK is blocked (dead card)
	SIMACCESS_NO_SIM,					///< No SIM
	SIMACCESS_MEMORY_EXCEEDED,			///< SIM Memmory exceeded
	SIMACCESS_CANNOT_REPLACE_SMS,		///< SIM can not replace sms
	SIMACCESS_BLOCKED_CHV2,             ///< CHV2 is blocked
	SIMACCESS_WARNING_CHV               ///< Warning in chv operation, e.g., not allowed to disable chv
} SIMAccess_t;							///< SIM access status


/// SIM Boolean Data
typedef struct
{
	SIMAccess_t result;		///< SIM access result
	Boolean		value;		///< function response.
} SIM_BOOLEAN_DATA_t;

/// SIM Integer Data
typedef struct
{
	SIMAccess_t result;		///< SIM access result
	UInt32		value;		///< function response.
} SIM_INTEGER_DATA_t;

/**
APDU File ID
**/
typedef enum
{
	APDUFILEID_INVALID_FILE		= 0x0000,

	// Master File
	APDUFILEID_MF				= 0x3F00,

	APDUFILEID_USIM_ADF			= 0x7FFF,
	APDUFILEID_ISIM_ADF			= 0x7FFF,

	APDUFILEID_DF_TELECOM		= 0x7F10,
	APDUFILEID_DF_GSM			= 0x7F20,

	/* DF's defined by Cingular for ENS (Enhanced Network Selection) feature */
	APDUFILEID_DF_CINGULAR		= 0x7F66,	///< APDUFILEID_DF_ACTNET & APDUFILEID_EF_CINGULAR_TST are under this DF 
	APDUFILEID_DF_ACTNET		= 0x5F30,	///< APDUFILEID_EF_ACTHPLMN is under this DF 

	/* USIM phonebook files: global phonebook under DF-Telecom and USIM-specific phonebook under USIM-ADF */
	APDUFILEID_DF_PHONEBK		= 0x5F3A,
	APDUFILEID_EF_PSC			= 0x4F22,	///< Phonebook synchronization counter
	APDUFILEID_EF_CC			= 0x4F23,	///< Change counter 
	APDUFILEID_EF_PUID			= 0x4F24,	///< Previous unique identifier
	APDUFILEID_EF_PBR			= 0x4F30,

	APDUFILEID_EF_ICCID			= 0x2FE2,
	APDUFILEID_EF_PL			= 0x2F05,	///< Called EF-ELP in GSM 11.11, but called EF-PL in 3GPP TS 51.011 

	APDUFILEID_EF_ARR_UNDER_MF	= 0x2F06,	///< EF-ERR under MF
	APDUFILEID_EF_ARR_UNDER_DF	= 0x6F06,	///< EF-ERR under ADF-USIM or DF-Telecom

	/* EF's defined by Cingular for ENS (Enhanced Network Selection) feature and RAT balancing feature */
	APDUFILEID_EF_ACTHPLMN		= 0x4F34,	///< Under APDUFILEID_DF_ACTNET DF 
	APDUFILEID_EF_RAT			= 0x4F36,	///< Under APDUFILEID_DF_ACTNET DF 
	APDUFILEID_EF_CINGULAR_TST	= 0x6FD2,	///< Under APDUFILEID_DF_CINGULAR DF 
	
	// DF Telecom
	APDUFILEID_EF_ADN			= 0x6F3A,
	APDUFILEID_EF_FDN			= 0x6F3B,
	APDUFILEID_EF_SMS			= 0x6F3C,
	APDUFILEID_EF_CCP			= 0x6F3D,
	APDUFILEID_EF_MSISDN		= 0x6F40,
	APDUFILEID_EF_SMSP			= 0x6F42,
	APDUFILEID_EF_SMSS			= 0x6F43,
	APDUFILEID_EF_LND			= 0x6F44,
	APDUFILEID_EF_SMSR			= 0x6F47,
	APDUFILEID_EF_SDN			= 0x6F49,
	APDUFILEID_EF_EXT1			= 0x6F4A,
	APDUFILEID_EF_EXT2			= 0x6F4B,
	APDUFILEID_EF_EXT3			= 0x6F4C,
	APDUFILEID_EF_BDN			= 0x6F4D,
	APDUFILEID_EF_EXT4			= 0x6F4E,
	APDUFILEID_EF_ECCP			= 0x6F4F,
	APDUFILEID_EF_CMI			= 0x6F58,

	APDUFILEID_ISIM_EF_IMPI		= 0x6F02,
	APDUFILEID_ISIM_EF_DOMAIN	= 0x6F03,
	APDUFILEID_ISIM_EF_IMPU		= 0x6F04,
	APDUFILEID_ISIM_EF_IST		= 0x6F07,
	APDUFILEID_ISIM_EF_AD		= 0x6FAD,
	APDUFILEID_ISIM_EF_ARR		= 0x6F06,
	APDUFILEID_ISIM_EF_P_CSCF	= 0x6F09,
	APDUFILEID_ISIM_EF_P_GBAP	= 0x6FD5,
	APDUFILEID_ISIM_EF_P_GBANL	= 0x6FD7,

	// DF GSM
	APDUFILEID_EF_LP			= 0x6F05,
	APDUFILEID_EF_IMSI			= 0x6F07,
	
    //DF Graphics
    APDUDILEID_DF_GRAPHICS      = 0x5F50,
    APDUFILEID_EF_IMG           = 0x4F20,

	/* The following EF are defined in Common PCN Handset 
	 * Sepcification (CPHS): under DF GSM. They are not defined in GSM 11.11.
	 */
	APDUFILEID_EF_MWI_CPHS		= 0x6F11,		///< voice message waiting
	APDUFILEID_EF_CFL_CPHS		= 0x6F13,		///< call forward flags
	APDUFILEID_EF_ONS_CPHS		= 0x6F14,		///< operator name string
	APDUFILEID_EF_CSP_CPHS		= 0x6F15,		///< customer service profile
	APDUFILEID_EF_INFO_CPHS		= 0x6F16,		///< CPHS info
	APDUFILEID_EF_MBN_CPHS		= 0x6F17,		///< mailbox numbers
	APDUFILEID_EF_ONSS_CPHS		= 0x6F18,		///< operator name string in shortform
	APDUFILEID_EF_IN_CPHS		= 0x6F19,		///< information numbers
	/* CPHS EF definitions end */

	APDUFILEID_EF_KC			= 0x6F20,
	APDUFILEID_EF_PLMNSEL		= 0x6F30,
	APDUFILEID_EF_HPLMN			= 0x6F31,
	APDUFILEID_EF_ACMMAX		= 0x6F37,
	APDUFILEID_EF_SST			= 0x6F38,
	APDUFILEID_EF_ACM			= 0x6F39,
	APDUFILEID_EF_GID1			= 0x6F3E,
	APDUFILEID_EF_GID2			= 0x6F3F,
	APDUFILEID_EF_PUCT			= 0x6F41,
	APDUFILEID_EF_CBMI			= 0x6F45,
	APDUFILEID_EF_SPN			= 0x6F46,
	APDUFILEID_EF_CBMID			= 0x6F48,
	APDUFILEID_EF_BCCH			= 0x6F74,
	APDUFILEID_EF_ACC			= 0x6F78,
	APDUFILEID_EF_FPLMN			= 0x6F7B,
	APDUFILEID_EF_LOCI			= 0x6F7E,
	APDUFILEID_EF_AD			= 0x6FAD,
	APDUFILEID_EF_PHASE			= 0x6FAE,
	APDUFILEID_EF_VGCS			= 0x6FB1,
	APDUFILEID_EF_VGCSS			= 0x6FB2,
	APDUFILEID_EF_VBS			= 0x6FB3,
	APDUFILEID_EF_VBSS			= 0x6FB4,
	APDUFILEID_EF_EMLPP			= 0x6FB5,
	APDUFILEID_EF_AAEM			= 0x6FB6,
	APDUFILEID_EF_ECC			= 0x6FB7,
	APDUFILEID_EF_CBMIR			= 0x6F50,

	
	APDUFILEID_EF_DCK			= 0x6F2C,

	APDUFILEID_EF_CNL			= 0x6F32,
	APDUFILEID_EF_UST_3G		= 0x6F38, /* EF-UST, exists only in USIM */

	APDUFILEID_EF_NIA			= 0x6F51,
	APDUFILEID_EF_KC_GPRS		= 0x6F52,
	APDUFILEID_EF_LOCI_GPRS		= 0x6F53,
	APDUFILEID_EF_SUME			= 0x6F54,
	APDUFILEID_EF_EST_3G		= 0x6F56, /* EF-EST, exists only in USIM */
	APDUFILEID_EF_ACL_3G		= 0x6F57, /* EF-ACL, exists only in USIM */
	APDUFILEID_EF_PLMN_WACT		= 0x6F60,
	APDUFILEID_EF_OPLMN_WACT	= 0x6F61,
	APDUFILEID_EF_HPLMN_WACT	= 0x6F62,
	APDUFILEID_EF_CPBCCH		= 0x6F63,
	APDUFILEID_EF_INV_SCAN		= 0x6F64,
	APDUFILEID_EF_RPLMN_ACT		= 0x6F65,

	APDUFILEID_EF_HIDDEN_KEY_3G = 0x6FC3,

	APDUFILEID_EF_PNN			= 0x6FC5,
	APDUFILEID_EF_OPL			= 0x6FC6,

	APDUFILEID_EF_MBDN			= 0x6FC7,
	APDUFILEID_EF_EXT6			= 0x6FC8,
	APDUFILEID_EF_MBI			= 0x6FC9,
	APDUFILEID_EF_MWIS			= 0x6FCA,
	APDUFILEID_EF_CFIS			= 0x6FCB,
	
	APDUFILEID_EF_EXT7			= 0x6FCC,
	APDUFILEID_EF_SPDI			= 0x6FCD,
	APDUFILEID_EF_MMSN			= 0x6FCE,
	APDUFILEID_EF_EXT8			= 0x6FCF,

	APDUFILEID_EF_MMSICP		= 0x6FD0,
	APDUFILEID_EF_MMSUP			= 0x6FD1,

	APDUFILEID_EF_MMSUCP		= 0x6FD2,

	/* If this is passed to the SIM API functions: the API functions obtain
	 * the Dedicated File ID based on the passed Elementary File ID.
	 */
	FIND_DFILE_ID_WITH_EFILE_ID = 0xFFFF

} APDUFileID_t;					///< SIM File ID

typedef UInt8 ICCID_BCD_t[ICC_DIGITS / 2];		///< ICC ID digits in BCD format
typedef UInt8 ICCID_ASCII_t[ICC_DIGITS + 1];	///< ICC ID string: null terminated

typedef UInt8 CPHS_ONS_t[CAPI2_MAX_CPHS_ONS_LEN]; ///< CPHS_ONS type
typedef UInt8 CPHS_ONSS_t[CAPI2_MAX_CPHS_ONSS_LEN]; ///< CPHS_ONSS type
typedef UInt8 SST_t[CAPI2_SST_SIZE];	///< SIM Service Table
typedef UInt8 CPHS_INFO_t[CAPI2_CPHS_INFO_SIZE];  ///< CPHS Info


typedef struct
{
	UInt32		length;					///< length of valid part of data buffer
	UInt8		data[261];				///< buffer for APDU data 
} APDU_t;


/**
T_UICCAP_CARD_TYPE in USIMAP to identify whether the SIM is running 2G or 3G application. 
**/
typedef enum
{
  SIM_APPL_2G,
  SIM_APPL_3G,
  SIM_APPL_INVALID
} SIM_APPL_TYPE_t;


/**
GSM 11.11 defines three types of EFiles *
**/
typedef enum
{
	TRANSPARENT_EFILE,
	LINEAR_FIXED_EFILE,
	CYCLIC_EFILE, 
	INVALID_EFILE
} EFILE_TYPE_t;

/** 
Access condition defined in Section 9.3 of GSM 11.11 
**/
typedef enum
{
	ALW_ACCESS_COND = 0x0,
	CHV1_ACCESS_COND = 0x1,
	CHV2_ACCESS_COND = 0x2,
	RFU_ACCESS_COND = 0x3, 
	ADM_LEV_4_ACCESS_COND = 0x4, 
	ADM_LEV_5_ACCESS_COND = 0x5, 
	ADM_LEV_6_ACCESS_COND = 0x6, 
	ADM_LEV_7_ACCESS_COND = 0x7, 
	ADM_LEV_8_ACCESS_COND = 0x8, 
	ADM_LEV_9_ACCESS_COND = 0x9, 
	ADM_LEV_10_ACCESS_COND = 0xA, 
	ADM_LEV_11_ACCESS_COND = 0xB, 
	ADM_LEV_12_ACCESS_COND = 0xC, 
	ADM_LEV_13_ACCESS_COND = 0xD, 
	ADM_LEV_14_ACCESS_COND = 0xE, 
	NEW_ACCESS_COND = 0xF
} EFILE_ACCESS_CONDITION_t;

/** 
Access condition defined in Section 7.1 of ISO/IEC 7816-4 and Section 9.5.1 of 3GPP 31.101. 
These access conditions are for 3G USIM only, not applicable for 2G SIM.
**/
typedef enum
{
	USIM_ALWAYS_ACCESS_COND				= 0x00,
	USIM_PIN1_APPLI1_ACCESS_COND		= 0x01,
	USIM_ADM1_ACCESS_COND				= 0x0A,
	USIM_ADM2_ACCESS_COND				= 0x0B,
	USIM_ADM3_ACCESS_COND				= 0x0C,
	USIM_ADM4_ACCESS_COND				= 0x0D,
	USIM_ADM5_ACCESS_COND				= 0x0E,
	USIM_ADM6_ACCESS_COND				= 0x8A,
	USIM_ADM7_ACCESS_COND				= 0x8B,
	USIM_ADM8_ACCESS_COND				= 0x8C,
	USIM_ADM9_ACCESS_COND				= 0x8D,
	USIM_ADM10_ACCESS_COND				= 0x8E,
	USIM_UNIVERSAL_PIN_ACCESS_COND		= 0x11,	/* Universal PIN to be used as PIN1 */	
	USIM_NEVER_ACCESS_COND				= 0x7F,
	USIM_PIN2_APPLI1_ACCESS_COND		= 0x81

	/* Since we always select the first application in USIM, other applications are not applicable.
	 * 
	 * USIM_PIN1_APPLI2_ACCESS_COND		= 0x02,
	 * USIM_PIN2_APPLI2_ACCESS_COND		= 0x82,
	 * USIM_PIN1_APPLI3_ACCESS_COND		= 0x03,
	 * USIM_PIN2_APPLI3_ACCESS_COND		= 0x83,
	 */
} USIM_EFILE_ACCESS_CONDITION_t;

/** 
CPHS Service Table entries 
**/
typedef enum
{
	OPNAME_SHORTFORM,
	MAILBOX_NUMBERS,
	SST,
	CSP,
	INFORMATION_NUMBERS
} CPHS_SST_ENTRY_t;



/** 
Service Code Status defined in CPHS
**/
typedef enum
{
	/* In service group CALL_OFFERING_CSP */
	CFU_CSP,				///< Unconditional call forward
	CFB_CSP,				///< Call forward when busy
	CFNRY_CSP,				///< Call forward when no reply
	CFNRC_CSP,				///< Call forward when not reachable
	CT_CSP,					// Call transfer
	
	/* In service group CALL_RESTRICTION_CSP */
	BOAC_CSP,				///< Barring of all outgoing call
	BOIC_CSP,				///< Barring of outgoing international call
	BOIC_EXHC_CSP,			///< Barring of outgoing international call except those directed to Home PLMN
	BAIC_CSP,				///< Barring of all incoming call
	BIC_ROAM_CSP,			///< Barring of incoming call when roaming outside Home PLMN

	/* In service group OTHER_SUPP_SERVICES_CSP */
	MPTY_CSP,				///< Multi-party call
	CUG_CSP,				///< Closed user group
	AOC_CSP,				///< Advice of charge
	PREF_CUG_CSP,			///< Perferential closed user group
	CUG_OA_CSP,				///< Closed user group outgoing access

	/* In service group CALL_COMPLETION_CSP */
	HOLD_CSP,				///< Call hold
	CW_CSP,					///< Call waiting
	CCBS_CSP,				///< Completion of call to busy subscriber
	USER_USERSIGNALING_CSP, ///< User to user signalling menu restriction

	/* In service group TELESERVICES_CSP */
	SMS_MT_CSP,				///< MT SMS
	SMS_MO_CSP,				///< MO SMS
	SMS_CB_CSP,				///< Cell broadcast message
	REPLY_PATH_CSP,			///< Restrict menu option to set reply path active on MO SMS
	DELIVERY_CONFIRM_CSP,	///< SMS Delivery confirmation
	PROTOCOL_ID_CSP,		///< Restriction of menu for SMS protocol ID option
	VALIDITY_PERIOD_CSP,	///< Restriction of menu for SMS validity period option

	/* In service group CPHS_TELESERVICES_CSP */
	ALS_CSP,				///< Alternative line service

	/* In service group NUMBER_IDENTIFICATION_CSP */
	CLIP_CSP,				///< Calling line identification presentation
	COLR_CSP,				///< Connected line identification restriction
	COLP_CSP,				///< Connected line identification presentation
	MCI_CSP,				///< Malicious call indicator
	CLI_SEND_CSP,			///< Calling line identfication per call mode, default block CLI
	CLI_BLOCK_CSP,			///< Calling line identfication per call mode, default send CLI

	/* In service group PHASE_2PLUS_SERVICES_CSP */
	GPRS_CSP,				///< Menu concerned with GPRS functionality
	HSCSD_CSP,				///< Menu concerned HSCSD
	VOICE_GROUPCALL_CSP,	///< ASCI voice group call menu
	VOICE_BROADCAST_CSP,	///< ASCI voice broadcast service menu
	MULTIPLE_SUBSCRIBER_CSP,///< Phase2+ multiple subscriber profile menu
	MULTIPLE_BAND_CSP,		///< Restriction of menu allowing user to select band

	/* In service group VALUE_ADDED_SERVICES_CSP */
	PLMN_MODE_CSP,			///< Restriction of menu allowing manual PLMN selection
	VPS_CSP,				///< Restriction of menu for voice mail or similar menus
	SMS_MO_PAGING_CSP,		///< Restriction of menu to send SMS with type paging
	SMS_MO_EMAIL_CSP,		///< Restriction of menu to send SMS with type email
	FAX_CSP,				///< Restriction of menu for fax call
	DATA_CSP,				///< Restriction of menu for data call
	LANGUAGE_CSP,			///< Restriction of menu to change language

	/* In service group INFORMATION_NUMBERS_CSP */
	INFORMATION_NO_CSP		///< Information numbers to user if set to 0xFF.
} SERVICE_CODE_STATUS_CPHS_t;

/**
Service Flag Status
**/
typedef enum
{
	SERVICE_FLAG_EXIST_CLEARED,	///< The corresponding bit exists in EF-CSP and is cleared 
	SERVICE_FLAG_EXIST_SET,		///< The corresponding bit exists in EF-CSP and is set 
	SERVICE_FLAG_NOT_EXIST		///< The corresponding bit does not exist in EF-CSP 
} SERVICE_FLAG_STATUS_t;

/**
  Status for Open Logical Channel request
**/
typedef struct
{
	UInt8 socket_id;		///< 0 if failure in opening logical channel; non-0 value otherwise
} SIM_OPEN_SOCKET_RES_t;

/**
  Status for Activate non-USIM Application request
**/
typedef struct
{
	SIMAccess_t result;		///< SIMACCESS_SUCCESS if non-USIM application is successfully activated
} SIM_SELECT_APPLI_RES_t;

/**
  Status for Deactivate non-USIM Application request
**/
typedef struct
{
	SIMAccess_t result;		///< SIMACCESS_SUCCESS if non-USIM application is successfully deactivated
} SIM_DEACTIVATE_APPLI_RES_t;

/**
  Status for Close Logical Channel request
**/
typedef struct
{
	SIMAccess_t result;		///< SIMACCESS_SUCCESS if logical channel is successfully closed
} SIM_CLOSE_SOCKET_RES_t;

/// Response parameters after selecting a MF/DF. See Section 9.2.1 of GSM 11.11 
typedef struct
{
	SIMAccess_t result;			///< SIM access result
	UInt16 mem_not_allocated;	///< Total memory not allocated under directory
	UInt8 file_charact;			///< File characteristics
	UInt8 num_df;				///< Number of DF's which are direct children of directory
	UInt8 num_ef;				///< Number of EF's which are direct children of directory
	UInt8 num_chv;				///< Number of unlock CHV codes, unblock CHV codes and adminstrative codes
	
	Boolean chv1_code_initialized;	///< TRUE if CHV1 code is initialized
	UInt8 chv1_unlock_attempt;		///< Number of remaining attempts to unlock CHV1
	UInt8 chv1_unblock_attempt;		///< Number of remaining attempts to unblock CHV1

	Boolean chv2_code_initialized;	///< TRUE if CHV2 code is initialized
	UInt8 chv2_unlock_attempt;		///< Number of remaining attempts to unlock CHV2
	UInt8 chv2_unblock_attempt;		///< Number of remaining attempts to unblock CHV2
}SIM_DFILE_INFO_t;

/* Life Cycle Status of a 3G EF: see Section 11.1.1.4.9 of 31.101 */ 
typedef enum
{
	NO_INFO_GIVEN_STATE,
	CREATION_STATE,
	INITIALIZATION_STATE,
	OPERATIONAL_ACTIVATED_STATE,
	OPERATIONAL_DEACTIVATED_STATE,
	TERMINATION_STATE
} USIM_LIFE_CYCLE_STATYS_t;



/// SIM EFile Information
typedef struct
{
	SIMAccess_t result;			///< SIM access result
	UInt16 file_size;			///< File size					
	UInt8 record_length;		///< record length (only for linear fixed or cyclic record)
	EFILE_TYPE_t efile_type;	///< Type: Transparent; linear Fixed; Cyclic

	/* Access condition defined in Section 9.3 of GSM 11.11: ONLY applicable for 2G SIM */
	EFILE_ACCESS_CONDITION_t update_access_cond;
	EFILE_ACCESS_CONDITION_t read_seek_access_cond;
	EFILE_ACCESS_CONDITION_t increase_access_cond;
	EFILE_ACCESS_CONDITION_t invalidate_access_cond;
	EFILE_ACCESS_CONDITION_t rehabilitate_access_cond;

	/* Access condition defined in Section 7.1 of ISO/IEC 7816-4 and Section 9.5.1 of 3GPP 31.101. 
	 * These access conditions are for 3G USIM only, not applicable for 2G SIM.
	 */
	USIM_EFILE_ACCESS_CONDITION_t	usim_update_access_cond;	/**< Include Update Binary & Update Record commands */
	USIM_EFILE_ACCESS_CONDITION_t	usim_read_access_cond;		/**< Include Read Binary, Read Record, Search Binary, Search Record commands */
	USIM_EFILE_ACCESS_CONDITION_t	usim_activate_access_cond;	
	USIM_EFILE_ACCESS_CONDITION_t	usim_deactivate_access_cond;

	/* Invalidation status: ONLY applicable for 2G SIM */
	Boolean validated;				///< TRUE if file is validated
	Boolean accessible_invalidated;	///< Readable & updatable when invalidated
	
	/** Life cycle status: ONLY applicable for 3G SIM */
	USIM_LIFE_CYCLE_STATYS_t life_cycle_satus;
} SIM_EFILE_INFO_t;

/// SIM E File Data
typedef struct
{
	SIMAccess_t result;		///< SIM access result
	UInt16		data_len;	///< Length of data returned.
	UInt8		*ptr;		///< Pointer to data buffer
} SIM_EFILE_DATA_t;

/// E File Update Result
typedef struct
{
	SIMAccess_t result;		///< SIM access result
} SIM_EFILE_UPDATE_RESULT_t;

/// SIM Search Record (Seek Command in 2G SIM) Response Data
typedef struct
{
	SIMAccess_t	result;			///< SIM access result
	UInt8		data_len;		///< Length of data returned.
	UInt8		*ptr;			///< Pointer to data buffer which stores 1-based record indices
} SIM_SEEK_RECORD_DATA_t;

/// General Data record structure
typedef struct
{
	SIMAccess_t result;		///< SIM access result
	UInt8	num_of_rec;		///< Number of records returned
	UInt8	rec_len;		///< Length of each record
	UInt8	*rec_data;		///< Dynamically allocated buffer storing record data
} SIM_GEN_REC_DATA_t;

/// Number of PIN/PUK attempts remaining
typedef struct
{
	SIMAccess_t result;		 ///< SIM access result
    UInt8 pin1_attempt_left; ///< Number of PIN 1 attempts remaining
    UInt8 pin2_attempt_left; ///< Number of PIN 2 attempts remaining
    UInt8 puk1_attempt_left; ///< Number of PUK 1 attempts remaining
    UInt8 puk2_attempt_left; ///< Number of PUK 2 attempts remaining
} PIN_ATTEMPT_RESULT_t;

/// Call Forward Unconditional status flags in CPHS EF-6F11 
typedef struct
{
	Boolean call_forward_l1;				///< Boolean indicating call forward on line 1
	Boolean call_forward_l2;				///< Boolean indicating call forward on line 2
	Boolean call_forward_fax;				///< Boolean indicating call forward on fax line
	Boolean call_forward_data;				///< Boolean indicating call forward on data line
} SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t;


/// SIM Call Forward Unconditional result
typedef struct
{
	SIMAccess_t result;								///< SIM access result
	Boolean		  sim_exist;						///< SIM exist?
	SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t	value;	///< function response.
} SIM_CALL_FORWARD_UNCONDITIONAL_STATUS_t;


/// SIM Service flag status
typedef struct
{
	SIMAccess_t result;						///< SIM access result
	SERVICE_FLAG_STATUS_t		value;		///< function response.
} SIM_SERVICE_FLAG_STATUS_t;

/// SIM ICCID result
typedef struct
{
	SIMAccess_t result;				///< SIM access result
	ICCID_ASCII_t		iccid;		///< ICC ID string: null terminated
} SIM_ICCID_STATUS_t;

typedef struct
{
	SIMAccess_t result;				///< SIM access result
	APDU_t atr_data;				///< ATR data
} SIM_ATR_DATA_t;

/** Service Centre Number as defined in SMSP Efile */
typedef UInt8 SIM_SCA_DATA_t[SMSPARAM_SCA_SZ];

/// SIM SCA Data result
typedef struct
{
	SIMAccess_t result;				///< SIM access result
	SIM_SCA_DATA_t		data;		///< Service Centre Number
} SIM_SCA_DATA_RESULT_t;

/// SIM SIM Application type result
typedef struct
{
	SIMAccess_t result;				///< SIM access result
	SIM_APPL_TYPE_t		type;		///< SIM Application type
} SIM_APPL_TYPE_RESULT_t;

/*************************Needed by SMS*********************************/
/**
SIM Message Waiting Indicator: do not change the following enum values. It is defined to adopt the record index defined in 2G CPHS spec. 
**/
typedef enum
{
	MWI_L1 = 1, 
	MWI_L2 = 2, 
	MWI_FAX = 3, 
	MWI_DATA = 4
} SIM_MWI_TYPE_t;

/**
Identifies the SIM/USIM files regarding PLMN
**/
typedef enum
{
	SIM_PLMN_SEL,		///< EF-PLMNsel applicable for 2G SIM only
	SIM_PLMN_WACT,		///< EF-PLMNwAct applicable for 2G SIM /3G USIM
	SIM_OPLMN_WACT,	///< EF-OPLMNwAct applicable for 2G SIM /3G USIM
	SIM_HPLMN_WACT     ///< EF-HPLMNwAct applicable for 2G SIM /3G USIM
} SIM_PLMN_FILE_t;

/// PLMN entry data. If empty in SIM/USIM, mcc/mnc=0xFFFF/0xFF. 
typedef struct
{
	UInt16		index;						///< 0-based index of the PLMN entry
	UInt16		mcc;						///< MCC which may include the 3rd digit of MNC, e.g. 13F0 for AT&T in Sunnyvale, CA
	UInt8		mnc;						///< MNC, e.g. 0x71 for AT&T in Sunnyvale, CA
	Boolean		gsm_act_selected;			///< TRUE if GSM Access Technology selected	(For 2G SIM, it is always FALSE)
	Boolean		gsm_compact_act_selected;	///< TRUE if GSM Compact Access Technology selected	(For 2G SIM, it is always FALSE)
	Boolean		utra_act_selected;			///< TRUE if UTRAN Access Technology selected (For 2G SIM, it is always FALSE)
} SIM_PLMN_ENTRY_t;

/// Number of PLMN entry response.
typedef struct
{
	SIMAccess_t result;				///< SIM access result
	SIM_PLMN_FILE_t plmn_file;		///< PLMN file
	UInt16 num_of_total_entry;		///< Number of total entries, including empty entries
} SIM_PLMN_NUM_OF_ENTRY_t;

/// Read PLMN entry response.
typedef struct
{
	SIMAccess_t result;				///< SIM access result
	SIM_PLMN_FILE_t plmn_file;		///< PLMN file
	UInt8 num_of_entry;				///< Number of entries in "plmn_entry", including empty entries
	SIM_PLMN_ENTRY_t plmn_entry[MAX_NUM_OF_PLMN];	///< PLMN entry data
} SIM_PLMN_ENTRY_DATA_t;

/// Update PLMN entry response.
typedef struct
{
	SIMAccess_t result;				///< SIM access result
	SIM_PLMN_FILE_t plmn_file;		///< PLMN file
	UInt16 index;					///< 0-based index to which the PLMN is written
} SIM_PLMN_ENTRY_UPDATE_t;

/// Update multiple PLMN entries response.
typedef struct
{
	SIMAccess_t result;				///< SIM access result
	SIM_PLMN_FILE_t plmn_file;		///< PLMN file
	UInt16 start_index;				///< 0-based index to which the start PLMN is written
	UInt16 num_of_entry;			///< number of PLMN entries that are written   
} SIM_MUL_PLMN_ENTRY_UPDATE_t;

/// PLMN data for writing multiple entries. If empty in SIM/USIM, mcc/mnc=0xFFFF/0xFF. 
typedef struct
{       
	UInt16		mcc;						///< MCC which may include the 3rd digit of MNC, e.g. 13F0 for AT&T in Sunnyvale, CA
	UInt8		mnc;						///< MNC, e.g. 0x71 for AT&T in Sunnyvale, CA
	Boolean		gsm_act_selected;			///< TRUE if GSM Access Technology selected     (For 2G SIM, it is always FALSE)
	Boolean		gsm_compact_act_selected;	///< TRUE if GSM Compact Access Technology selected     (For 2G SIM, it is always FALSE)
	Boolean		utra_act_selected;			///< TRUE if UTRAN Access Technology selected (For 2G SIM, it is always FALSE)
} SIM_MUL_PLMN_ENTRY_t;


/*************************Needed by Phonebook*********************************/
/** Non-alpha data length in a SIM phonebook EF, e.g. EF-ADN, EF-LND, EF-SDN */
#define SIM_PBK_NON_ALPHA_SZ			14

#define SIM_PBK_DIGIT_SZ				10
#define SIM_PBK_ASCII_DIGIT_SZ			(SIM_PBK_DIGIT_SZ * 2)

#define ECC_DIGITS						6	///< Maximum digits in emergency call code
#define ECC_ENTRY_SIZE	 (ECC_DIGITS / 2)	///< Number of bytes of each emergency call code

/** Number of bytes in a record of EF-EXT1, EF-EXT2 etc */
#define SIM_EXT_DIGIT_NUM_OF_BYTES	13

/** Number of bytes used for storing EXT digits */
#define SIM_PBK_EXT_BCD_DIGIT_SIZE (SIM_EXT_DIGIT_NUM_OF_BYTES - 3)

/** Max number of extension digits in EF-EXT1 and EF-EXT2 etc */
#define SIM_PBK_EXT_ASCII_DIGIT_SIZE (SIM_PBK_EXT_BCD_DIGIT_SIZE * 2)

/** Emergency Call Code: BCD coded */
typedef UInt8 ECC_NUM_t[ECC_ENTRY_SIZE];

typedef enum
{
    ECC_SRC_MS,
    ECC_SRC_SIM,
    ECC_SRC_NW,
    ECC_SRC_UNKNOWN = 0xFF
}ECC_SRC_t;

typedef struct
{
  ECC_NUM_t ecc_num;
  UInt8 emergency_svc_category;
  ECC_SRC_t ecc_src;
} ECC_REC_t;

typedef ECC_REC_t ECC_REC_LIST_t[CAPI2_ECC_REC_LIST_SIZE]; 


//******************************************************************************
//							Constant Definitions
//******************************************************************************
#define CHV_MIN_LENGTH					4	///< Min. number of digits in CHV
#define CHV_MAX_LENGTH					8	///< Max. number of digits in CHV, PUK
#define PUK_MIN_LENGTH					8	///< Min. number of digits in PUK
#define PUK_MAX_LENGTH					8	///< Min. number of digits in PUK
#define SIMPBK_NAME						40	///< Max. phone book name
#define SIMPBK_DIGITS					40	///< Max. phone book number
#define IMSI_DIGITS						15	///< Max. IMSI digits
#define GID_DIGITS						10	///< Max. GID1/GID2 file length
#define SVC_PROV_SIZE					16	///< Service Provider string size
#define CURRENCY_SIZE					3	///< Currency string size 





//******************************************************************************
//					Enumeration Definitions
//******************************************************************************

/** 
Common command values defined in Section 10.1.2 of 3GPP 31.101 & Section 9.2 of GSM 11.11 
**/
typedef enum
{
	APDUCMD_SELECT				= 0xA4,
	APDUCMD_STATUS				= 0xF2,
	
	APDUCMD_READ_BINARY			= 0xB0,
	APDUCMD_UPDATE_BINARY		= 0xD6,
	APDUCMD_READ_RECORD			= 0xB2,
	APDUCMD_UPDATE_RECORD		= 0xDC,
	
	APDUCMD_SEEK				= 0xA2,	///< Called "Seek" command in GSM, but called "Search Record" in USIM 
	APDUCMD_INCREASE			= 0x32,

	APDUCMD_VERIFY_CHV			= 0x20,	///< Command to verify CHV
	APDUCMD_CHANGE_CHV			= 0x24,	///< Change CHV command
	APDUCMD_DISABLE_CHV			= 0x26,	///< Command to disable CHV
	APDUCMD_ENABLE_CHV			= 0x28, ///< Enable CHV command
	APDUCMD_UNBLOCK_CHV			= 0x2C, ///< Unblock CHV command

	APDUCMD_INVALIDATE			= 0x04,	///< Called "Invalidate" in GSM, but called "Deactivate File" in USIM 
	APDUCMD_REHABILITATE		= 0x44, ///< Called "Rehabilitate" in GSM, but called "Activate File" in USIM 
	
	APDUCMD_AUTHENTICATE		= 0x88, ///< Called "Run GSM Algorithm" in GSM, but called "Authenticate" in USIM 

	APDUCMD_SLEEP				= 0xFA,	///< This is old GSM command, not valid in USIM 

	APDUCMD_TERMINAL_PROFILE	= 0x10,
	APDUCMD_ENVELOPE			= 0xC2,
	APDUCMD_FETCH				= 0x12,
	APDUCMD_TERMINAL_RESPONSE	= 0x14,
	APDUCMD_MANAGE_CHANNEL		= 0x70,

	APDUCMD_GET_RESPONSE		= 0xC0
} APDUCmd_t;

/**
APDU Class
**/
typedef enum
{
	APDUCLASS_2G_A0				= 0xA0,
	APDUCLASS_3G_00				= 0x00,
	APDUCLASS_3G_80				= 0x80
} APDUClass_t;


/**
CHV type
**/
typedef enum
{
	CHV1 = SI_CHV1,
	CHV2 = SI_CHV2
} CHV_t;

/**
SIM present status
**/
typedef enum
{
	SIMPRESENT_REMOVED = SI_SIM_NOT_PRESENT,	///< SIM Romoved
	SIMPRESENT_INSERTED = SI_SIM_PRESENT,	///< SIM Inserted
	SIMPRESENT_NO_STATUS = SI_SIM_NOT_SET,	///< No Status Available yet
	SIMPRESENT_ERROR = SI_SIM_ERROR			///< No Status Available yet
} SIMPresent_t;


/**
Type of SIM
**/
typedef enum
{
	SIMTYPE_NORMAL_SIM 		= NORMAL_SIM,
	SIMTYPE_NORMAL_SPECIAL_SIM = NORMAL_SPECIAL_SIM,
	SIMTYPE_APPROVAL_SIM 	= APPROVAL_SIM,
	SIMTYPE_APPROVAL_SPECIAL_SIM = APPROVAL_SPECIAL_SIM,
	SIMTYPE_SERVICE_CARD_SIM = SERVICE_CARD_SIM,
	SIMTYPE_CELL_TEST_SIM 	= CELL_TEST_SIM,
	SIMTYPE_NO_CARD 		= NO_CARD
} SIMType_t;

/**
SIM Phase
**/
typedef enum
{
	SIMPHASE_1,
	SIMPHASE_2,
	SIMPHASE_2PLUS
} SIMPhase_t;


/**
SIM phonebook ID
**/
typedef enum
{
	SIMPBK_ADN,							///< Abbreviated Dialing Number Phonebook
	SIMPBK_FDN,							///< Fixed Dialing Number Phonebook
	SIMPBK_SDN,							///< Service Dialing Number Phonebook
	SIMPBK_LND,							///< Last Number Dialed Phonebook
	SIMPBK_MSISDN,						///< MS ISDN Dialing Number Phonebook
    SIMPBK_BDN                          ///< Barred Dialing Number Phonebook
} SIMPBK_ID_t;							///< SIM Phonebook ID


/**
PLMN action type
**/
typedef enum
{
	SIMPLMNACTION_INSERT = SI_INSERT,
	SIMPLMNACTION_DELETE = SI_DELETE
} SIMPLMNAction_t;

/**
SIM PIN status
**/
typedef enum
{
	PIN_READY_STATUS,				///< ME is not pending for any password
	SIM_PIN_STATUS,					///< ME is waiting SIM PIN to be given
	SIM_PUK_STATUS,					///< ME is waiting SIM PUK to be given
	PH_SIM_PIN_STATUS,				///< ME is waiting phone-to-SIM card password to be given
	PH_SIM_PUK_STATUS,				///< Phone-to-SIM card password blocked
	SIM_PIN2_STATUS,				///< SIM PIN2 status
	SIM_PUK2_STATUS,				///< SIM PUK2 status
	PH_NET_PIN_STATUS,				///< Network Lock PIN to be given
	PH_NET_PUK_STATUS,				///< Network Lock PUK to be given
	PH_NETSUB_PIN_STATUS,			///< Network Subset Lock PIN to be given
	PH_NETSUB_PUK_STATUS,			///< Network Subset Lock PUK to be given
	PH_SP_PIN_STATUS,				///< Service Provider Lock PIN to be given
	PH_SP_PUK_STATUS,				///< Service Provider Lock PUK to be given
	PH_CORP_PIN_STATUS,				///< Corporate Lock PIN to be given
	PH_CORP_PUK_STATUS,				///< Corporate Lock PUK to be given
	NO_SIM_PIN_STATUS,

	///< SIM related PIN status
	SIM_ABSENT_STATUS,				///< SIM is not inserted
	SIM_ERROR_STATUS,				///< SIM operation error: should never get this status if everything works	
	SIM_PUK_BLOCK_STATUS,			///< SIM is permanently blocked because SIM PUK attempt exceeded
	SIM_PUK2_BLOCK_STATUS,			///< SIM PIN2 is permanently blocked because PUK2 attempt exceeded, but normal UE function is OK
} SIM_PIN_Status_t;


//******************************************************************************
//							Type Definitions
//******************************************************************************

// Maximum number of bytes to read/write data in Binary file because of APDU limit
#define MAX_BINARY_FILE_DATA_LEN_PER_CMD	255


#define MAX_BINARY_FILE_DATA_LEN   (MAX_BINARY_FILE_DATA_LEN_PER_CMD * 6)    ////< Maximum number of bytes for SIM Binary File access

 

typedef UInt8 CHVString_t[CHV_MAX_LENGTH + 1]; ///< CHV string
typedef UInt8 PUKString_t[PUK_MAX_LENGTH + 1]; ///< PUK string

typedef UInt8 SIMPBK_Name_t[SIMPBK_NAME]; ///< Alpha string associated with number
										
typedef UInt8 SIMPBK_Number_t[SIMPBK_DIGITS + 1]; ///< Phonebook number, NULL terminated

/// SIM phonebook entry
typedef struct
{
	gsm_TON_t ton;						///< Type of number
	gsm_NPI_t npi;						///< Numbering plan identifier
	SIMPBK_Name_t name;					///< Alpha string associated with number
	SIMPBK_Number_t digits;				///< Phonebook number
	UInt8	ext_rec_index;				///< 0-based index of record in EF-EXT1, EF-EXT2 etc.
    UInt8 cmp;                          ///< Comparison Method Pointer
} SIMPBK_Entry_t;						///< SIM Phonebook entry


typedef UInt8 IMSI_t[ IMSI_DIGITS + 1 ];///< IMSI ID string
typedef UInt8 GID_DIGIT_t[GID_DIGITS];	///< GID1/GID2 file length (not null terminated)

typedef UInt8 SvcProvName_t[ SVC_PROV_SIZE + 1 ]; ///< Service provider name (null-term.)
										
typedef UInt8 CurrencyName_t[ CURRENCY_SIZE + 1 ]; ///< Currency string (null-term)
										
/// EPPU
typedef struct
{
	UInt16 mant;	///< EPPU mantissa
	Int16 exp;		///< EPPU exponent (signed)
} EPPU_t;
										
/**
SIM Message Status
**/
typedef enum
{
	SIMSMSMESGSTATUS_FREE		= 0x00,	///< Status Free
	SIMSMSMESGSTATUS_READ 		= 0x01,	///< Status Read
	SIMSMSMESGSTATUS_UNREAD 	= 0x03,	///< Status Unread
	SIMSMSMESGSTATUS_SENT	 	= 0x05,	///< Status Sent
	SIMSMSMESGSTATUS_UNSENT 	= 0x07	///< Status Unsent
} SIMSMSMesgStatus_t;					///< SIM SMS Message Status


#define	SMSMESG_DATA_SZ		175


/// SIM Message Type
typedef struct
{	// See GSM 11.11, Section 10.3.3
	SIMSMSMesgStatus_t 	status;			///< SMS Message Status
	UInt8 mesg_data[ SMSMESG_DATA_SZ ];	///< SMS Message data 15 extra bytes to take care of SIMAP segmenting
} SIMSMSMesg_t;



//******************************************************************************
//							Structure Typedefs
//******************************************************************************


typedef enum
{
	SIMOPERSTATE_NO_OPERATION	= SI_NO_OPERATION,
	SIMOPERSTATE_RESTRICTED_OPERATION = SI_RESTRICTED_OPERATION,
	SIMOPERSTATE_UNRESTRICTED_OPERATION = SI_UNRESTRICTED_OPERATION
} SIMOperState_t;

typedef enum
{
	SIMSERVICESTATUS_NOT_ALLOCATED1	= 0,	///< Service Status is unknown
	SIMSERVICESTATUS_NOT_ACTIVATED	= 1,	///< Service Status is allocated, but not activated
	SIMSERVICESTATUS_NOT_ALLOCATED2	= 2,	///< Service Status is not allocated
	SIMSERVICESTATUS_ACTIVATED		= 3		///< Service Status is allocated and activated
} SIMServiceStatus_t;

/* Note: Do not change the order of the following enum. The order is associated with
 *       the coding in function SIM_GetServiceStatus() in sim_mi.c.
 */
/**
Services provided by the SIM
**/
typedef enum
{
	SIMSERVICE_CHV1,						///< CHV1 Disable
	SIMSERVICE_ADN,							///< ADN
	SIMSERVICE_FDN,							///< FDN
	SIMSERVICE_SMSM,						///< SMS Message
	SIMSERVICE_AOC,							///< Advice of Charge
	SIMSERVICE_CCP,							///< Capability Configuration Parameters
	SIMSERVICE_PLMN_SEL,					///< PLMN Selector
	SIMSERVICE_RFU1,						///< not used

	// the following are optional, so some SIM may not contain these services
	SIMSERVICE_MSISDN,						///< MSISDN
	SIMSERVICE_EXT1,						///< Extension1
	SIMSERVICE_EXT2,						///< Extension2
	SIMSERVICE_SMSP,						///< SMS Parameter
	SIMSERVICE_LND,							///< LND
	SIMSERVICE_CBMI,						///< Cell Broadcast Message Identifier
	SIMSERVICE_GID1,						///< Group Identifier Level 1
	SIMSERVICE_GID2,						///< Group Identifier Level 2
	SIMSERVICE_SPN,							///< Service Provider Name
	SIMSERVICE_SDN,							///< SDN
	SIMSERVICE_EXT3,						///< Extension3
	SIMSERVICE_RFU2,						///< not used
	SIMSERVICE_VGCS_VGCSS,					// 
	SIMSERVICE_VBS_VBSS,					// 
	SIMSERVICE_EMLPP,						///< enhanced Multi-Level Precedence and Pre-emption
	SIMSERVICE_AAEM,						///< Automatic Answer EMLPP
	SIMSERVICE_DDSMSCB,						///< Data Download SMS Cell Broadcast
	SIMSERVICE_DDSMSPP,						///< Data Download SMS Point-Point
	SIMSERVICE_MSEL,						///< Menu Selection
	SIMSERVICE_CCNTL,						///< Call Control
	SIMSERVICE_PSIM,						///< Proactive SIM
	SIMSERVICE_CBMIP,						///< Cell Broadcast Message Identifier Parameters
	SIMSERVICE_BDN,							///< Barred Dialling Numbers
	SIMSERVICE_EXT4,						///< Extension 4
	SIMSERVICE_DCK,							///< De-personalization control keys
	SIMSERVICE_CNL,							///< Co-operative Network list
	SIMSERVICE_SMSR,						///< Short message status reports
	SIMSERVICE_ALERT,						///< Network indication of alerting in MS
	SIMSERVICE_MOSMS_CCNTL,					///< MO SMS control by SIM
	SIMSERVICE_GPRS,						///< GPRS
	SIMSERVICE_IMG,							///< Image(IMG)
	SIMSERVICE_SOLSA,						///< Support of local service area
	SIMSERVICE_USSD_OBJ,					///< USSD string data object supported in call control
	SIMSERVICE_AT_CMD,						///< Run AT COMMAND command
	SIMSERVICE_USR_PLMN_SEL,				///< User controlled PLMN selector with access technology
	SIMSERVICE_OPR_PLMN_SEL,				///< Operator controlled PLMN selector with access technology
	SIMSERVICE_HOME_PLMN_SEL,				///< Home PLMN selector with access technology 
	SIMSERVICE_CPBCCH_INFO,					///< CPBCCH information
	SIMSERVICE_INV_SCAN,					///< Investigation scan
	SIMSERVICE_EXT_CONF,					///< Extended capability configuration parameters
	SIMSERVICE_MEXE,						///< MExE
	SIMSERVICE_RPLMN_LAST_USED,				///< RPLMN last used access technology
	SIMSERVICE_PNN,							///< PLMN network name 
	SIMSERVICE_OPL,							///< Operator PLMN list
	SIMSERVICE_MBDN,						///< Mailbox dialing numbers
	SIMSERVICE_MWIS,						///< Message waiting indication status
	SIMSERVICE_CFIS,						///< Call forward indication status
	SIMSERVICE_SP_DIS,						///< Service provider display indication
	SIMSERVICE_MMS,							///< Multi-media messaging service (MMS)
	SIMSERVICE_EXT8,						///< Extension 8
	SIMSERVICE_MMS_UCP,						///< MMS User connectivity parameters

	SIMSERVICE_LOCAL_PHONEBK,				///< Exists only for USIM
	SIMSERVICE_OUTGOING_CALL_INFO,			///< Exists only for USIM
	SIMSERVICE_INCOMING_CALL_INFO,			///< Exists only for USIM
	SIMSERVICE_CBMIR,						///< Exists only for USIM: Cell Broadcast Identifier Range
	SIMSERVICE_GSM_ACCESS,					///< Exists only for USIM
	SIMSERVICE_ENABLED_SERVICE_TABLE,		///< Exists only for USIM
	SIMSERVICE_APN_CONTROL_LIST,			///< Exists only for USIM: APN Control List (ACL)
	SIMSERVICE_GSM_SECURITY_CONTEXT,		///< Exists only for USIM
	SIMSERVICE_EXT5,						///< Exists only for USIM: Extension 5
	SIMSERVICE_GPRS_CALL_CONTROL,			///< Exists only for USIM: Call Control on GPRS

	SIMSERVICE_RFU

} SIMService_t;



//******************************************************************************
// Callback Definitions
//******************************************************************************
/// SIM Phonebook Information
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	SIMPBK_ID_t pbk;					///< Phonebook
	UInt16 total;						///< Total number of entries in phone book
	UInt16 alpha_sz;					///< Size of alpha part of entry
} SIM_PBK_INFO_t;

/// SIM Phonebook write result
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	SIMPBK_ID_t pbk;					///< Phonebook
	UInt16		index;					///< Write index
	SIMPBK_Entry_t	simpbk_entry;		///< Phonebook contents written
	void		*additional_pbk_cb;		///< Additional phonebook callback function
} SIM_PBK_WRITE_RESULT_t;

/// SIM Phonebook Data
typedef struct
{
	SIMAccess_t		result;				///< SIM access result
	SIMPBK_ID_t		pbk;				///< Phonebook
	UInt16			index;				///< Current record index
	UInt16			end_index;			///< Index of the last record to read
	SIMPBK_Entry_t	buffer;				///< Phonebook contents
	void			*additional_pbk_cb;	///< Additional phonebook callback function
} SIM_PBK_DATA_t;

/// SIM Maximum accumulated call meter structure
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	CallMeterUnit_t max_acm;			///< Max value of the ACM
} SIM_MAX_ACM_t;

/// SIM ACM value
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	CallMeterUnit_t acm;				///< Value of the ACM
} SIM_ACM_VALUE_t;

/// SIM Service Provider name
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	Boolean display_flag1;				///< 2G SIM:TRUE, registered PLMN display is mandatory,
                                        ///< 3G USIM: TRUE,mandatory if registered PLMN is HPLMN or in Service Provider List                              
    Boolean display_flag2;				///< 2G SIM: N/A, 3G USIM:TRUE, Service Provide display is not required
                                        ///< if registered PLMN is HPLMN or in Service Provider List    
	SvcProvName_t name;					///< Pointer to buffer to place name in
} SIM_SVC_PROV_NAME_t;

/// SIM PUCT Data
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	CurrencyName_t currency;			///< 3-char currency code string, NULL terminated
	EPPU_t eppu;						///< Elementary Price Per Unit
} SIM_PUCT_DATA_t;


/// SIM SMS Update Result
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	UInt16		rec_no;					///< Record number found, range 0 to n-1
} SIM_SMS_UPDATE_RESULT_t;

/// SIM SMS Data
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	UInt16		rec_no;					///< Record number found, 0 to n-1
	SIMSMSMesg_t sms_mesg;				///< pointer to SMS Message
} SIM_SMS_DATA_t;

/// Restricted SIM access data
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	UInt8 *data;						///< The data returned from SIM if available
	UInt16 data_len;					///< data length
	UInt8 sw1;							///< status word
	UInt8 sw2;							// status word
} SIM_RESTRICTED_ACCESS_DATA_t;

/// Generic SIM access data
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	UInt8 *data;						///< The data returned from SIM if available
	UInt16 data_len;					///< data length
} SIM_GENERIC_ACCESS_DATA_t;


/// SIM access phase result
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	SIMPhase_t  phase;					///< The data returned from SIM if available
} SIM_PHASE_RESULT_t;

/// SIM type result
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	SIMType_t  type;					///< SIM Type
} SIM_TYPE_RESULT_t;

/// SIM present result
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	SIMPresent_t  status;					///< SIM status
} SIM_PRESENT_RESULT_t;

/// SIM service status result
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	SIMServiceStatus_t	status;			///< Service Status
} SIM_SERVICE_STATUS_RESULT_t;

/// SIM pin status result
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	SIM_PIN_Status_t	pin_status;		///< Pin Status
}SIM_PIN_STATUS_RESULT_t;

/// SIM imsi result
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	IMSI_t	imsi;					///<IMSI ID string
}SIM_IMSI_RESULT_t;

/// SIM gid string result
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	GID_DIGIT_t	gid;					///<GID1/GID2 digit string
}SIM_GID_RESULT_t;

/// SIM home plmn result
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	PLMNId_t	plmn;					///<plmn
}SIM_HOME_PLMN_RESULT_t;

/// SIM apdu file result
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	APDUFileID_t	simFileId;					///<SIM File ID
}SIM_APDU_FILEID_RESULT_t;


/// SIM result
typedef struct
{
	SIMAccess_t result;					///< SIM access result
}SIM_ACCESS_RESULT_t;


typedef enum {	/* First SIMLOCK type must start with 0 and subsequent SIMLOCK 
				 * types must be consecutive. Code implementation assumes this.
				 */ 
	SIMLOCK_NETWORK_LOCK = 0, 
	SIMLOCK_NET_SUBSET_LOCK,
	SIMLOCK_PROVIDER_LOCK,
	SIMLOCK_CORP_LOCK,
	SIMLOCK_PHONE_LOCK,

	SIMLOCK_INVALID_LOCK /* This must be put in the last */

} SIMLockType_t;


#define MAX_SIM_SIG_DATA		20
#define MAX_SIM_IMEI_DATA		8

/** Status of unlocking Network or Service Provider lock */
typedef enum
{
	SIMLOCK_SUCCESS,			/* Lock successfully unlocked */
	SIMLOCK_FAILURE,			/* Lock not unlocked, but can try again */
	SIMLOCK_PERMANENTLY_LOCKED,	/* Lock not unlocked, trying again will fail even if correct
								 * control key is provided because we have exceeded the 
								 * maximum number of trials allowed*/	
	SIMLOCK_WRONG_KEY,
	SIMLOCK_NOT_ALLOWED,
	SIMLOCK_DISABLED,

	SIMLOCK_INVALID_STATUS

} SIMLock_Status_t;

/// SIM Lock ON status
typedef struct
{
	SIMAccess_t result;		///< SIM access result
	Boolean		phsLock;	///< TRUE if PH-SIM full lock is on.
	Boolean		simLock;	///< TRUE - the lock is on; FALSE - lock is off.
} SIM_LOCK_ON_RESULT_t;

/// SIM Lock status
typedef struct
{
	SIMAccess_t				result;		///< SIM access result
	SIMLock_Status_t		lockStatus;	///< SIM lock status
} SIM_LOCK_RESULT_t;

/// SIM Lock Type status
typedef struct
{
	SIMAccess_t			result;			///< SIM access result
	Boolean				lockBlocked;	///< SIM block status
	SIMLockType_t		lockType;		///< SIM lock type
} SIM_LOCK_TYPE_RESULT_t;


/// SIM Lock signature data
typedef struct
{
	SIMAccess_t	result;						///< SIM access result
	UInt8		data[MAX_SIM_SIG_DATA];		///< SIM signature data
} SIM_LOCK_SIG_DATA_t;

/// SIM Lock imei data
typedef struct
{
	SIMAccess_t result;						///< SIM access result
	UInt8		data[MAX_SIM_IMEI_DATA];	///< SIM imei data
} SIM_LOCK_IMEI_DATA_t;

/**
USIM EF-UST data: USIM Service Table
**/
#define MAX_UST_LEN	15

/// SIM UST Data
typedef struct
{
	SIMAccess_t result;
	UInt8 Ust_Data[MAX_UST_LEN];
}USIM_UST_DATA_RSP_t;

/**
SIM BDN Operation state
**/
typedef enum
{
	SIMBDNSTATE_NO_OPERATION	= SI_NO_OPERATION,
	SIMBDNSTATE_RESTRICTED_OPERATION = SI_RESTRICTED_OPERATION,
	SIMBDNSTATE_UNRESTRICTED_OPERATION = SI_UNRESTRICTED_OPERATION
} SIMBdnOperState_t;

/**
	Tag length for each HomeZone tag ID 
**/
#define SIM_HZ_TAGS_LEN	12	


/**
 SIMLOCK status for a SIMLOCK type
 **/
typedef enum
{
	SIM_SECURITY_OPEN,		///< SIMLOCK/PhoneLock verified or disabled
	SIM_SECURITY_LOCKED,	///< SIMLOCK/PhoneLock enabled & pending on password verification
	SIM_SECURITY_BLOCKED	///< SIMLOCK/PhoneLock blocked & may or may not be unblocked (this status is implementation dependent)
} SIM_SECURITY_STATE_t;


/// SIMLOCK status for all SIMLOCK types
typedef struct
{
	SIM_SECURITY_STATE_t network_lock;
	SIM_SECURITY_STATE_t network_subset_lock;
	SIM_SECURITY_STATE_t service_provider_lock;
	SIM_SECURITY_STATE_t corporate_lock;
	SIM_SECURITY_STATE_t phone_lock;
} SIMLOCK_STATE_t;


/// SIMLOCK status for all SIMLOCK types
typedef struct
{
	IMSI_t imsi_string;	///< NULL terminated IMSI string
	GID_DIGIT_t	gid1;	///< GID1 data
	UInt8 gid1_len;		///< Number of bytes in "gid1" element, i.e. number of bytes in EF-GID1
	GID_DIGIT_t gid2;	///< GID2 data
	UInt8 gid2_len;		///< Number of bytes in "gid2" element, i.e. number of bytes in EF-GID2
} SIMLOCK_SIM_DATA_t;

/**
 Maximum number of languages in SIM files: EF-ELP & EF-LP in 2G SIM; EF-LI & EF-PL in 3G USIM.
 **/
#define MAX_NUM_OF_SIM_LANGUAGE	15

/// Preferred Language Setting in 2G SIM. Section 11.2.1 of GSM 11.11 defines language preference in 2G SIM. 
typedef struct
{
	UInt16	elp_language_list[MAX_NUM_OF_SIM_LANGUAGE];	///< EF-ELP language list in ISO-639 encoding: each language is represented by a pair of alpha-numberic characters, e.g. "EN" (0x454E) for English. 0xFFFF or 0x0000 if not defined
	UInt8	lp_language_list[MAX_NUM_OF_SIM_LANGUAGE];	///< EF-LP language list in CB lauguage encoding: defined in Section 5 of GSM 03.38. Each byte defines a language, e.g. 0x01 for English. 0xFF if not defined
} SIM_2G_LANGUAGE_t;


/// Preferred Language Setting in 3G USIM. Section 5.1.1.2 of 3GPP 31.102 defines language preference in 3G USIM.
typedef struct
{
	UInt16	li_language_list[MAX_NUM_OF_SIM_LANGUAGE];	///< EF-LI language list in ISO-639 encoding: each language is represented by a pair of alpha-numberic characters, e.g. "EN" (0x454E) for English. 0xFFFF or 0x0000 if not defined
	UInt16	pl_language_list[MAX_NUM_OF_SIM_LANGUAGE];	///< EF-PL language list in ISO-639 encoding: each language is represented by a pair of alpha-numberic characters, e.g. "EN" (0x454E) for English. 0xFFFF or 0x0000 if not defined
} SIM_3G_LANGUAGE_t;

/// Preferred Language Setting union structure in SIM
typedef union
{
	SIM_2G_LANGUAGE_t	language_2g_sim;	///< Applicable if 2G SIM inserted
	SIM_3G_LANGUAGE_t	language_3g_usim;	///< Applicable if 3G SIM inserted
} SIM_LANGUAGE_INFO_t;

/// SIM Detection Information
typedef struct
{
	SIM_APPL_TYPE_t		sim_appl_type;		///< Inserted SIM type: SIM_APPL_INVALID if SIM not inserted; otherwise SIM_APPL_2G or SIM_APPL_3G
	SIM_LANGUAGE_INFO_t	lang_info;			///< SIM language list: applicable only if SIM inserted (sim_appl_type is SIM_APPL_2G or SIM_APPL_3G)
	Boolean				ruim_supported;		///< TRUE if the inserted SIM is RUIM (CDMA2000 SIM)
} SIM_DETECTION_t;

/*
  Generic ADPU definitions
*/
#define SIM_GENERIC_APDU_MAX_ATR_LEN 34
#define SIM_GENERIC_APDU_MAX_CMD_LEN 256
#define SIM_GENERIC_APDU_MAX_RSP_LEN 256

typedef enum
{
    SIM_POWER_ON_GENERIC_MODE,
    SIM_POWER_ON_NORMAL_MODE,
    SIM_POWER_ON_INVALID_MODE = 0xff
}SIM_POWER_ON_MODE_t;

typedef enum
{
    SIM_GENERIC_APDU_RES_SUCCESS,
    SIM_GENERIC_APDU_RES_FAILURE,
    SIM_GENERIC_APDU_RES_NOT_ACCESSABLE,
    SIM_GENERIC_APDU_RES_POWERED_OFF,
    SIM_GENERIC_APDU_RES_REMOVED,
    SIM_GENERIC_APDU_RES_POWERED_ON,
    SIM_GENERIC_APDU_RES_NOT_AVAIL
}SIM_GENERIC_APDU_RESULT_t;

typedef enum
{
   SIM_GENERIC_APDU_POWER_ON_OFF_EVT = 1,
   SIM_GENERIC_APDU_CARD_STATUS = 2,
   SIM_GENERIC_APDU_ATR_RESP = 3,
   SIM_GENERIC_APDU_XFER_RESP = 4
}SIM_GENERIC_APDU_EVENT_t;

typedef struct
{
    SIM_GENERIC_APDU_EVENT_t eventType;
    SIM_GENERIC_APDU_RESULT_t resultCode;
}SIM_GENERIC_APDU_RES_INFO_t;

typedef struct
{
    SIM_GENERIC_APDU_EVENT_t eventType;
    SIM_GENERIC_APDU_RESULT_t resultCode;
    UInt8 len;
    UInt8 data[SIM_GENERIC_APDU_MAX_ATR_LEN];
}SIM_GENERIC_APDU_ATR_INFO_t;

typedef struct
{
    SIM_GENERIC_APDU_RESULT_t resultCode;
    UInt16 len;
    UInt8 data[SIM_GENERIC_APDU_MAX_RSP_LEN];
}SIM_GENERIC_APDU_XFER_RSP_t;

typedef UInt8  SIM_GENERIC_APDU_CMD_BUF_t[SIM_GENERIC_APDU_MAX_CMD_LEN];

/// SIM fatal error condition
typedef enum
{
	SIM_IMSI_NOT_EXIST,		///< Mandatory EF-IMSI does not exist. MMI shall indicate invalid SIM to user, but can choose to continue to access other SIM files, such as phonebook
	SIM_CONTINUAL_TECH_RPOBLEM	///< SIM continally sends SW1/SW2=0x6F/0x00 tech problem status, i.e. AT&T Phonebook feature
} SIM_FATAL_ERROR_t;

/**
 * @addtogroup CAPI2_SIMAPIGroup
 * @{
 */

//-------------------------------------------------
// Function Prototype
//-------------------------------------------------

void CAPI2_SIM_SendOpenSocketReq(UInt32 tid, UInt8 clientID);

void CAPI2_SIM_SendSelectAppiReq(UInt32 tid, UInt8 clientID, UInt8 socket_id, const UInt8 *aid_data, UInt8 aid_len);

void CAPI2_SIM_SendDeactivateAppiReq(UInt32 tid, UInt8 clientID, UInt8 socket_id);

void CAPI2_SIM_SendCloseSocketReq(UInt32 tid, UInt8 clientID, UInt8 socket_id);

void CAPI2_SIM_GetAtrData(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
    This function request the parameter information of a MF/DF in the SIM, i.e. the response
	data to the Select MF/DF command.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		dfile_id (in) Data File ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_DFILE_INFO_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_DFILE_INFO_t
**/	

void CAPI2_SIM_SubmitDFileInfoReq(UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t dfile_id, UInt8 path_len,
	const UInt16 *select_path);


//***************************************************************************************
/**
    This function request the basic information of an EF in the SIM, i.e. the size of the file, and
	the length of a record (if the file is linear fixed or cyclic).
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_EFILE_INFO_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_EFILE_INFO_t
**/	

void CAPI2_SIM_SubmitEFileInfoReq(UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len,
	const UInt16 *select_path);


//***************************************************************************************
/**
    This function request all the contents of a transparent EF in the SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_EFILE_DATA_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_EFILE_DATA_t
**/	

void CAPI2_SIM_SubmitWholeBinaryEFileReadReq(UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id,
								  UInt8 path_len, const UInt16 *select_path );

//***************************************************************************************
/**
    This function request some data in a transparent EF in the SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		offset (in) offset of the first byte to be read in the EF.
	@param		length (in) number of bytes to read in the EF (must not be larger than MAX_BINARY_FILE_DATA_LEN)
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		None
	@note		It is OK to pass "offset" and "data_len" arguments that are larger than
		  		the APDU limit. Internally this function will send multiple Read Binary
				commands if the passed "offset" and "data_len" arguments exceed the APDU limit. 
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_EFILE_DATA_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_EFILE_DATA_t
**/	

void CAPI2_SIM_SubmitBinaryEFileReadReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
						     UInt16 length, UInt8 path_len, const UInt16 *select_path );


//***************************************************************************************
/**
    This function request the contents of a record in a linear fixed or cyclic EF in the SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		rec_no (in) one-based record number.
	@param		length (in) length of a record in the EF.
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_EFILE_DATA_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_EFILE_DATA_t
**/	

void CAPI2_SIM_SubmitRecordEFileReadReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, 
							 UInt8 length, UInt8 path_len, const UInt16 *select_path);


//***************************************************************************************
/**
    This function request to update the contents of a transparent EF in the SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		offset (in) offset of the first byte to be written in the EF.
	@param		*ptr (in) pointer to the data to be written.
	@param		length (in) number of bytes to be written in the EF (must not be larger than MAX_BINARY_FILE_DATA_LEN)
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		None
	@note		It is OK to pass "offset" and "data_len" arguments that are larger than
		  		the APDU limit. Internally this function will send multiple Update Binary
				commands if the passed "offset" and "data_len" arguments exceed the APDU limit. 

	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_EFILE_UPDATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_EFILE_UPDATE_RESULT_t
**/	

void CAPI2_SIM_SubmitBinaryEFileUpdateReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
							   const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path);


//***************************************************************************************
/**
    This function request to update the contents of a record in a linear fixed EF in the SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		rec_no (in) one-based record number.
	@param		*ptr (in) pointer to the data to be written.
	@param		length (in) number of bytes to be written in the EF.
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_EFILE_UPDATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_EFILE_UPDATE_RESULT_t
**/	

void CAPI2_SIM_SubmitLinearEFileUpdateReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
				UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path ); 


//***************************************************************************************
/**
    This function requests to send the Search Record command (called Seek command in 2G SIM spec 11.11)
	to the SIM. For 3G USIM, it gets all the record indices of a linear fixed or cyclic file that matches a given pattern.
	For 2G SIM, it gets the index of the first record in a linear fixed or cyclic file that matches a given pattern.

	It should be used for only 2G SIM or 3G USIM application. 

    This function is very useful for speeding up SIM file cacheing on 3G USIM. For example in 3G phonebook in USIM, 
	typically a lot of the empty records exist in the phonebook files, e.g. EF-ADN, EF-PBC and EF-GRP. 

	This function can be called once to get indix list of the empty records for the above three exmplary files:
	EF-ADN: pass the pattern data of all 0xFF to get indices of empty records. 
	EF-PBC: pass the pattern data of all 0x00 to get indices of empty records.
	EF-GRP: pass the pattern data of all 0x00 to get indices of empty records. 
	
	Once the empty record list is obtained, no Read Record command is sent for the empty records but we can just 
	memset our data buffers to 0xFF or 0x00. Thus this can greatly reduce phonebook initialization time. 

	Besides phonebook files, this function can be used for any linear fixed or cyclic files, e.g. EONS EF-PNN, 
	EONS EF-OPL, EF-FDN & EF-SDN. 

	The above techniques shall be used for 3G USIM only. It is not useful for 2G SIM because the Seek command in 
	2G SIM returns the indix of the first matching record instead of the indices of all the matching records. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) Elementary File ID
	@param		dfile_id (in) Directory File ID
	@param		*ptr (in) pointer to data pattern
	@param		length (in) length of data pattern
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		Result_t
	@note
	The SIM task returns the result in the ::MSG_SIM_SEEK_REC_RSP message by
	calling the passed callback function.
**/	
void CAPI2_SIM_SubmitSeekRecordReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
							    const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path );


//***************************************************************************************
/**
    This function request to update the contents of the next record in a cyclic EF in the SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		*ptr (in) pointer to the data to be written.
	@param		length (in) length of record in the EF.
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_EFILE_UPDATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_EFILE_UPDATE_RESULT_t
**/	

void CAPI2_SIM_SubmitCyclicEFileUpdateReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
							   const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);


//***************************************************************************************
/**
    This function request the number of remaining PIN1 and PIN2 attempts in the SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_PIN_ATTEMPT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : PIN_ATTEMPT_RESULT_t
**/	

void CAPI2_SIM_SendRemainingPinAttemptReq(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
    This function request cached data status in SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_CACHE_DATA_READY_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_BOOLEAN_DATA_t
**/	
void CAPI2_SIM_IsCachedDataReady(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function request the service bit status in the Customer Service Profile (EF-CSP).	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		service_code (in) Service Code
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_SERVICE_CODE_STATUS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_SERVICE_FLAG_STATUS_t
**/	
void CAPI2_SIM_GetServiceCodeStatus(UInt32 tid, UInt8 clientID, SERVICE_CODE_STATUS_CPHS_t service_code);



//***************************************************************************************
/**
	This function request the status of CPHS service status from SIM	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		sst_entry (in) CPHS entry
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_CHECK_CPHS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_BOOLEAN_DATA_t
**/	
void CAPI2_SIM_CheckCphsService(UInt32 tid, UInt8 clientID, CPHS_SST_ENTRY_t sst_entry);

//***************************************************************************************
/**
	This function request the SIM CPHA Phase status	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_CPHS_PHASE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_INTEGER_DATA_t
**/	
void CAPI2_SIM_GetCphsPhase(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	This function request the Service Center Number in the SMSP EF in the SIM.	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		rec_no (in) record number
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n = RESULT_PENDING : The request is accepted and response is sent later as follows.
	@n@b MsgType_t : MSG_SIM_SMS_SCA_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR, SMS_SIM_BUSY, SMS_SIM_NOT_INSERT
	@n@b ResultData : SIM_SCA_DATA_RESULT_t
**/	
void CAPI2_SIM_GetSmsSca(UInt32 tid, UInt8 clientID, UInt8 rec_no);


//***************************************************************************************
/**
	This function request the SMS record number from SIM	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_PARAM_REC_NUM_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_INTEGER_DATA_t
**/	
void CAPI2_SIM_GetSmsParamRecNum(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	This function request the ICCID param
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_ICCID_PARAM_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_ICCID_STATUS_t
**/	
void CAPI2_SIM_GetIccid(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	This function request the SIM status of the user subscribtion to Alternative Line
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_ALS_STATUS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_BOOLEAN_DATA_t
**/	
void CAPI2_SIM_IsALSEnabled(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	This function request current selected default voice line (L1 or L2 of ALS).
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_ALS_DEFAULT_LINE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_INTEGER_DATA_t (UInt8 : 0 if L1 is selected, 1 if L2 is selected)
	@note
	Upon powerup, L1 is the default line, but the client can change the line by calling
	SIM_SetAlsDefaultLine() function. Note that upon power recycle, the default line reverts
	back to L1 because the line selection is saved in SRAM.
**/	
void CAPI2_SIM_GetAlsDefaultLine(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function request selection of default voice line (L1 or L2 of ALS).
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		line (in) 0 for L1; 1 for L2.
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_SET_ALS_DEFAULT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : None
	@note
	Upon powerup, L1 is the default line, but the client can change the line by calling
	SIM_SetAlsDefaultLine() function. Note that upon power recycle, the default line reverts
	back to L1 because the line selection is saved in SRAM.
**/	
void CAPI2_SIM_SetAlsDefaultLine(UInt32 tid, UInt8 clientID, UInt8 line);

//***************************************************************************************
/**
	This function request call forward status and also sim file existence status
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_CALLFWD_COND_FLAG_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t
**/	
void CAPI2_SIM_GetCallForwardUnconditionalFlag(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function request application type
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_APP_TYPE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_APPL_TYPE_RESULT_t
**/	
void CAPI2_SIM_GetApplicationType(UInt32 tid, UInt8 clientID);

//******************************************************************************
//     The following functions are only valid when the SIM is inserted
//******************************************************************************

//***************************************************************************************
/**
    This function sends request to check on whether SIM PIN is required
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PIN_REQ_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_BOOLEAN_DATA_t  TRUE, if PIN is required for operations
**/	
void CAPI2_SIM_IsPINRequired( UInt32 tid, UInt8 clientID);		


//***************************************************************************************
/**
    This function sends request to check on SIM card phase
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_CARD_PHASE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_PHASE_RESULT_t
**/	
void CAPI2_SIM_GetCardPhase( UInt32 tid, UInt8 clientID );


//******************************************************************************
// The following functions are used for SIM security
//******************************************************************************


//***************************************************************************************
/**
    This function sends request to check on SIM card type
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_TYPE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_TYPE_RESULT_t
**/	
void CAPI2_SIM_GetSIMType( UInt32 tid, UInt8 clientID );			// Get SIM type


//***************************************************************************************
/**
    This function sends request to check on SIM present status
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PRESENT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_PRESENT_RESULT_t
**/	
void CAPI2_SIM_GetPresentStatus( UInt32 tid, UInt8 clientID );	// SIM present status


//***************************************************************************************
/**
    This function sends request to check on SIM Operation response
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PIN_OPERATION_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_BOOLEAN_DATA_t  TRUE, if SIM operation is restricted
**/	
void CAPI2_SIM_IsOperationRestricted( UInt32 tid, UInt8 clientID );// TRUE, if SIM is inserted, but restricted

//***************************************************************************************
/**
    This function sends request to check on whether SIM is blocked
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chv (in) chv type
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PIN_BLOCK_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_BOOLEAN_DATA_t  TRUE, if CHV PIN is blocked
**/	
void CAPI2_SIM_IsPINBlocked(UInt32 tid, UInt8 clientID, CHV_t chv);	//  Check if CHV's PIN blocked

//***************************************************************************************
/**
    This function sends request to check on whether SIM PUK is blocked
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chv (in) chv type
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PUK_BLOCK_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_BOOLEAN_DATA_t  TRUE, if PUK is blocked
**/	
void CAPI2_SIM_IsPUKBlocked(UInt32 tid, UInt8 clientID, CHV_t chv);	//  Check if PUK blocked (unblock attempts exceeded

//***************************************************************************************
/**
    This function sends request to check if SIM is invalid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_IS_INVALID_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_BOOLEAN_DATA_t  TRUE, if SIM is invalid
**/	
void CAPI2_SIM_IsInvalidSIM(UInt32 tid, UInt8 clientID);			//  Check if the inserted SIM/USIM is invalid


//***************************************************************************************
/**
  This function returns whether SIM is inserted. It is typically used for simple SIM detection test in prodution line. 
  This function is different from CAPI2_SIM_GetPresentStatus(). CAPI2_SIM_GetPresentStatus() works in Flight or Normal mode, 
  not in Off and Charging mode. CAPI2_SIM_DetectSim() works in Flight, Noraml, Off and Charging mode.

  @param		tid (in) Unique exchange/transaction id which is passed back in the response
  @param		clientID (in) Client ID
  @return		None
  @note
  The Async SIM module response is as follows
  @n@b Responses 
  @n@b MsgType_t :	MSG_SIM_DETECT_RSP
  @n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
  @n@b ResultData :	SIM_BOOLEAN_DATA_t  TRUE if SIM is inserted; FALSE otherwise
**/
void CAPI2_SIM_DetectSim(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
  This function returns TRUE if a RUIM card is inserted; otherwise it returns FALSE. 

  @param		tid (in) Unique exchange/transaction id which is passed back in the response
  @param		clientID (in) Client ID
  @return		None
  @note
  The Async SIM module response is as follows
  @n@b Responses 
  @n@b MsgType_t :	MSG_SIM_GET_RUIM_SUPP_FLAG_RSP
  @n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
  @n@b ResultData :	SIM_BOOLEAN_DATA_t  TRUE if SIM is inserted; FALSE otherwise
**/
void CAPI2_SIM_GetRuimSuppFlag(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
    This function sends request to verify chv
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chv_select (in) CHV selected
	@param		chv (in) chv string
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_VERIFY_CHV_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_ACCESS_RESULT_t
**/	
void CAPI2_SIM_SendVerifyChvReq(			
	UInt32 tid,								 
	UInt8 clientID,						 
	CHV_t chv_select,					
	CHVString_t chv);


//***************************************************************************************
/**
    This function sends request to verify chv
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chv_select (in) CHV selected
	@param		old_chv (in) chv string
	@param		new_chv (in) chv string
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_CHANGE_CHV_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_ACCESS_RESULT_t
**/	
void CAPI2_SIM_SendChangeChvReq(			
	UInt32 tid,								 
	UInt8 clientID,						 
	CHV_t chv_select,					
	CHVString_t old_chv,				
	CHVString_t new_chv);

//***************************************************************************************
/**
    This function sends request to verify chv
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chv (in) attempted CHV (null-term.)
	@param		enable_flag (in) TRUE, SIM password enabled
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_ENABLE_CHV_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_ACCESS_RESULT_t
**/	
void CAPI2_SIM_SendSetChv1OnOffReq(		
	UInt32 tid,								 
	UInt8 clientID,						 
	CHVString_t chv,					
	Boolean enable_flag);

//***************************************************************************************
/**
    This function sends request to unblock chv
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chv_select (in) CHV selected
	@param		puk (in) attempted PUK (null-term.)
	@param		new_chv (in) attempted new CHV (null-term.)
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_UNBLOCK_CHV_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_ACCESS_RESULT_t
**/	
void CAPI2_SIM_SendUnblockChvReq(			
	UInt32 tid,								 
	UInt8 clientID,						 
	CHV_t chv_select,					
	PUKString_t puk,					
	CHVString_t new_chv);

//***************************************************************************************
/**
    This function sends request to set operation state
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		oper_state (in) FDN state setting
	@param		chv2 (in) chv string
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_SET_FDN_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_ACCESS_RESULT_t
**/	
void CAPI2_SIM_SendSetOperStateReq( 
	UInt32 tid,								 
	UInt8 clientID,						 
	SIMOperState_t oper_state,			
	CHVString_t chv2);


//******************************************************************************
// SIM Phonebook Functions (ADN, SDN, FDN, LND, and MS-ISDN phonebooks)
//******************************************************************************

//***************************************************************************************
/**
    This function sends request to check if SIM is invalid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		id (in) Phonebook id
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_IS_PBK_ALLOWED_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_BOOLEAN_DATA_t  TRUE, if SIM Phone Book is allowed
**/	
void CAPI2_SIM_IsPbkAccessAllowed(		// returns if access to Phonebook is allowed
	UInt32 tid,								 
	UInt8 clientID,						 
	SIMPBK_ID_t id					// Phonebook in question
	);


//***************************************************************************************
/**
    This function sends phone book info request
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		id (in) Phonebook id
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PBK_INFO_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_PBK_INFO_t
**/	
void CAPI2_SIM_SendPbkInfoReq(			
	UInt32 tid,								 
	UInt8 clientID,						 
	SIMPBK_ID_t id);



//******************************************************************************
//							GSM File Access
//******************************************************************************


//***************************************************************************************
/**
    This function sends request to update preferred plmn list
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		plmn (in) PLMN value to insert	or delete
	@param		action (in) action to take
	@return		None
	@note		even to delete the PLMN, index and the PLMN value must match, 
				otherwise the delete will fail.
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PLMN_WRITE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_ACCESS_RESULT_t
**/
void CAPI2_SIM_SendUpdatePrefListReq(		
	UInt32 tid,								 
	UInt8 clientID,						 
	PLMN_ID_t *plmn,
	SIMPLMNAction_t action);

//***************************************************************************************
/**
    This function sends request to read acm max request
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_MAX_ACM_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_MAX_ACM_t
**/
void CAPI2_SIM_SendReadAcmMaxReq(			
	UInt32 tid,								 
	UInt8 clientID						 
	);

//***************************************************************************************
/**
    This function sends request to write maximum Accumulated Call Meter value.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acm_max (in) Accumulated Call Meter
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_ACM_MAX_UPDATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_ACCESS_RESULT_t
**/
void CAPI2_SIM_SendWriteAcmMaxReq(		
	UInt32 tid,								 
	UInt8 clientID,						 
	CallMeterUnit_t acm_max);

//***************************************************************************************
/**
    This function sends request to read Accumulated Call Meter value.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_ACM_VALUE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_ACM_VALUE_t
**/
void CAPI2_SIM_SendReadAcmReq(
	UInt32 tid,								 
	UInt8 clientID						 
	);

//***************************************************************************************
/**
    This function sends request to write Accumulated Call Meter value.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acm (in) Accumulated Call Meter
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_ACM_UPDATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_ACCESS_RESULT_t
**/
void CAPI2_SIM_SendWriteAcmReq(	
	UInt32 tid,								 
	UInt8 clientID,						 
	CallMeterUnit_t acm);

//***************************************************************************************
/**
    This function sends request to increase Accumulated Call Meter value.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acm (in) Accumulated Call Meter
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_ACM_INCREASE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_ACCESS_RESULT_t
**/
void CAPI2_SIM_SendIncreaseAcmReq(		
	UInt32 tid,								
	UInt8 clientID,						
	CallMeterUnit_t acm);

//***************************************************************************************
/**
    This function sends request to read the Service Provider Name in SIM in EF-SPN.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_SVC_PROV_NAME_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_SVC_PROV_NAME_t
**/
void CAPI2_SIM_SendReadSvcProvNameReq(	
	UInt32 tid,								
	UInt8 clientID						
	);

//***************************************************************************************
/**
    This function sends request to read Price Per Unit and Currency information in SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PUCT_DATA_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_PUCT_DATA_t
**/
void CAPI2_SIM_SendReadPuctReq(			
	UInt32 tid,								
	UInt8 clientID						
	);

//***************************************************************************************
/**
    This function sends request to update Price Per Unit and Currency information in SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		currency (in) 3-char currency code string, NULL terminated
	@param		eppu (in) Elementary Price Per Unit
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PUCT_UPDATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_ACCESS_RESULT_t
**/
void CAPI2_SIM_SendWritePuctReq(			
	UInt32 tid,								
	UInt8 clientID,						
	CurrencyName_t currency,			
	EPPU_t *eppu);


//******************************************************************************
//						Miscellaneous Commands
//******************************************************************************

//***************************************************************************************
/**
    This function sends request SIM Service Status for a specific service
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		service (in) service type
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_SERVICE_STATUS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_SERVICE_STATUS_RESULT_t
**/
void CAPI2_SIM_GetServiceStatus(UInt32 tid, UInt8 clientID, SIMService_t service); // Get Service Status

//***************************************************************************************
/**
    This function sends request SIM generic access.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		data (in) PDU Data
	@param		data_length (in) PDU Length
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_GENERIC_ACCESS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_GENERIC_ACCESS_DATA_t
**/
void CAPI2_SIM_SendGenericAccessReq(UInt32 tid, UInt8 clientID, UInt16 data_length, UInt8 *data);

//***************************************************************************************
/**
    This function sends request terminate the generic SIM access session. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_GENERIC_ACCESS_END_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_SIM_SendGenericAccessEndReq(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function request SIM restricted access.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		command (in) SIM command
	@param		efile_id (in) SIM file id
	@param		dfile_id (in) Parent DF file id
	@param		p1 (in) instruction param 1
	@param		p2 (in) instruction param 2
	@param		p3 (in) instruction param 3
	@param		path_len (in) Number of file ID's in "select_path". 
	@param		select_path (in) Select path for the parent DF of "file_id", starting with MF (0x3F00)
	@param		data (in) PDU Data
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_RESTRICTED_ACCESS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_RESTRICTED_ACCESS_DATA_t
**/
void CAPI2_SIM_SubmitRestrictedAccessReq(				
	UInt32 tid,								
	UInt8 clientID,						
	UInt8 socket_id,
	APDUCmd_t command,					
	APDUFileID_t efile_id,				
	APDUFileID_t dfile_id,				
	UInt8 p1,							
	UInt8 p2,							
	UInt8 p3,							
	UInt8 path_len,						
	const UInt16 *select_path,			
	const UInt8 *data);


//***************************************************************************************
/**
    This function sends request current SIM PIN status
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PIN_STATUS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_PIN_STATUS_RESULT_t
**/
void CAPI2_SIM_GetPinStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function sends request if the SIM PIN status allows normal operation.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PIN_OK_STATUS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_BOOLEAN_DATA_t
**/
void CAPI2_SIM_IsPinOK(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function sends request the location of IMSI data. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_IMSI_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_IMSI_RESULT_t
**/
void CAPI2_SIM_GetIMSI(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function sends request the location of GID1 data.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_GID_DIGIT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_GID_RESULT_t
**/
void CAPI2_SIM_GetGID1(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function sends request the location of GID2 data.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_GID_DIGIT_RSP
	@n@b Result_t :		Result_t::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_GID_RESULT_t
**/
void CAPI2_SIM_GetGID2(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function sends request the PLMN structure for Home PLMN with the appropriate MCC and MNC values
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_HOME_PLMN_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_HOME_PLMN_RESULT_t
**/
void CAPI2_SIM_GetHomePlmn(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function sends request the ID of the parent file (MF or DF) for an EF.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		file_id (in) SIM File ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_APDU_FILEID_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_APDU_FILEID_RESULT_t
**/
void CAPI2_simmi_GetMasterFileId(UInt32 tid, UInt8 clientID, APDUFileID_t file_id);

//***************************************************************************************
/**
    This function sends request the SIM PIN status corresponding to the current locked SIMLOCK type. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SIM_PIN_STATUS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	EType SIM_PIN_STATUS_RESULT_t
	@note
			SIM PIN status corresponding to the current locked SIMLOCK type if 
 		    there is a locked SIMLOCK type. 
 		   "PIN_READY_STATUS" if there is no locked SIMLOCK type.
**/
void CAPI2_SIM_GetCurrLockedSimlockType(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function request SIM Lock ON status
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		lockType (in) network lock, provider lock, and etc.
	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_LOCK_ON_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_LOCK_ON_RESULT_t
**/	
void CAPI2_SIMLockIsLockOn(UInt32 tid, UInt8 clientID, SIMLockType_t lockType);


//***************************************************************************************
/**
	This function checks all lock status
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		imsi (in) Client ID
	@param		gid1 (in) GID1 file, pass NULL if SIM not inserted
	@param		gid2 (in) GID2 file. pass NULL if SIM not inserted
	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_LOCK_STATUS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_BOOLEAN_DATA_t
**/
void CAPI2_SIMLockCheckAllLocks(UInt32 tid, UInt8 clientID, UInt8* imsi, UInt8* gid1, UInt8* gid2);

//***************************************************************************************
/**
	This function checks all unlock status
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		lockType (in) network lock, provider lock, and etc.
	@param		key (in) key
	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_LOCK_UNLOCK_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_LOCK_RESULT_t
**/
void CAPI2_SIMLockUnlockSIM(UInt32 tid, UInt8 clientID, SIMLockType_t lockType, UInt8* key);


//***************************************************************************************
/**
	This function request SIM lock set lock/unlock (indicator on/off)
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		action (in) action - 1: lock SIM; 0 = unlock SIM
	@param		ph_sim_full_lock_on (in) ph_sim_full_lock_on - whether PH-SIM full lock should be set on.
	@param		lockType (in) network lock, provider lock, and etc.
	@param		key (in) control key
	@param		imsi (in) imsi - IMSI (MCC and MNC)
	@param		gid1 (in) GID1 file
	@param		gid2 (in) GID2 file
	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_LOCK_SET_LOCK_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_LOCK_RESULT_t
**/
void CAPI2_SIMLockSetLock(UInt32 tid, 
								 UInt8 clientID, 
								 UInt8 action, 
								 Boolean ph_sim_full_lock_on, 
								 SIMLockType_t lockType,  
								 UInt8* key, 
								 UInt8* imsi, 
								 UInt8* gid1, 
								 UInt8* gid2);  

//***************************************************************************************
/**
	This function request current closed lock type
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_LOCK_CLOSED_LOCK_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_LOCK_TYPE_RESULT_t
**/
void CAPI2_SIMLockGetCurrentClosedLock(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	This function request change in PH-SIM lock password
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		old_pwd (in) the old ASCII coded PH-SIM password (null terminated)
	@param		new_pwd (in) the new ASCII coded PH-SIM password (null terminated)
	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_LOCK_CHANGE_PWD_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_LOCK_RESULT_t
**/
void CAPI2_SIMLockChangePasswordPHSIM(UInt32 tid, UInt8 clientID, UInt8 *old_pwd, UInt8 *new_pwd);

//***************************************************************************************
/**
	This function checks the PH-SIM lock password
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		pwd (in) the ASCII coded PH-SIM password (null terminated)
	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_LOCK_CHECK_PWD_PHSIM_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_BOOLEAN_DATA_t
**/
void CAPI2_SIMLockCheckPasswordPHSIM(UInt32 tid, UInt8 clientID, UInt8 *pwd);

// Return the sim lock signature
//***************************************************************************************
/**
	This function request lock signature
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_LOCK_LOCK_SIG_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_LOCK_SIG_DATA_t
**/
void CAPI2_SIMLockGetSignature(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function request Imei in Secure Bootloader
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_LOCK_GET_IMEI_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_LOCK_IMEI_DATA_t
**/
void CAPI2_SIMLockGetImeiSecboot(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function updates SMS Memory Exceeded flag in SIM
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param      cap_exceeded (in) flag to update
	@return		None
	@note
	@n@b Responses 

**/

void CAPI2_SIM_UpdateSMSCapExceededFlag(UInt32 tid, UInt8 clientID, Boolean cap_exceeded);

//***************************************************************************************
/**
	This function gets SMS Memory Exceeded flag in SIM
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	@n@b Responses 

**/

void CAPI2_SIM_GetSmsMemExceededFlag(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	This function requests the number of PLMN entries in the PLMN file. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		plmn_file (in) PLMN file to access
	@return		None
	@note
	The SIM module result is as follows based on interim async response
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_PLMN_NUM_OF_ENTRY_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_PLMN_NUM_OF_ENTRY_t
**/	
void CAPI2_SIM_SendNumOfPLMNEntryReq(UInt32 tid, UInt8 clientID, SIM_PLMN_FILE_t plmn_file);

//***************************************************************************************
/**
	This function requests PLMN entry contents in the PLMN file. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		plmn_file (in) PLMN file to access
	@param		start_index (in) 0-based index of the first entry requested. 
	@param		end_index (in) 0-based index of the last entry requested. Pass 
				UNKNOWN_PLMN_INDEX to read all including the last entry starting from "start_index". 
	@return		None
	@note
	The SIM module result is as follows based on interim async response
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_PLMN_ENTRY_DATA_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_PLMN_ENTRY_DATA_t
**/	
void CAPI2_SIM_SendReadPLMNEntryReq( UInt32 tid, UInt8 clientID, SIM_PLMN_FILE_t plmn_file, 
				UInt16 start_index, UInt16 end_index );

//***************************************************************************************
/**
	This function requests to update a PLMN entry in the PLMN file. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		plmn_file (in) PLMN file to access
	@param		index (in) 0-based index of the entry to write. Pass UNKNOWN_PLMN_INDEX to write to the first free entry.
	@param		mcc (in) MCC in raw format, e.g. pass 0x13F0 for AT&T in Sunnyvale, CA
	@param		mnc (in) MNC in raw format, e.g. pass 0x71 for AT&T in Sunnyvale, CA
	@param		gsm_act_selected (in) TRUE if GSM Access Technology selected (ignored for 2G SIM)
	@param		gsm_compact_act_selected (in) TRUE if GSM Compact Access Technology selected (ignored for 2G SIM)
	@param		utra_act_selected (in) TRUE if UTRAN Access Technology selected (ignored for 2G SIM)
	@return		None
	@note
	The SIM module result is as follows based on interim async response
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_PLMN_ENTRY_UPDATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_PLMN_ENTRY_UPDATE_t

	This function is obsolete. Please use CAPI2_SIM_SendWriteMulPLMNEntryReq() instead!!!!!!
**/	
void CAPI2_SIM_SendWritePLMNEntryReq( UInt32 tid, UInt8 clientID, SIM_PLMN_FILE_t plmn_file, UInt16 index,
	UInt16 mcc, UInt8 mnc, Boolean gsm_act_selected, Boolean gsm_compact_act_selected, Boolean utra_act_selected);

//***************************************************************************************
/**	
    This function requests to update multiple PLMN entries in the PLMN file.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		plmn_file (in) PLMN file to access, SIM_PLMN_SEL or SIM_PLMN_WACT for 2G SIM and
	            SIM_PLMN_WACT for 3G USIM
    @param      first_idx (in) 0-based index of the first entry to write. If UNKNOWN_PLMN_INDEX
				is passed, the entries will be written to the free entry slots in SIM file. If 
				there are not enough free slots, error will be returned.
    @param      number_of_entry (in) Number of consecutive entries to write
    @param      plmn_entry[] (in) PLMN data entries to write
    @return		None
	@note
	The SIM module result is as follows based on interim async response
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_MUL_PLMN_ENTRY_UPDATE_t
	
**/
void CAPI2_SIM_SendWriteMulPLMNEntryReq( UInt32 tid, UInt8 clientID, SIM_PLMN_FILE_t plmn_file, UInt16 first_idx,
	UInt16 number_of_entry, SIM_MUL_PLMN_ENTRY_t plmn_entry[]);

/** @} */

#define MAX_APN_NAME_LEN 100

typedef char APN_NAME_t[MAX_APN_NAME_LEN + 1];	///< Index for adding a new APN name to the EF-ACL

/**
 Activated/Deactivated status for services defined in EF-EST. 
 The following enum's must be defined in the same order as that in Section 4.2.47 of 31.102.
**/
typedef enum
{
	USIM_EST_FDN_SERVICE,		
	USIM_EST_BDN_SERVICE,
	USIM_EST_ACL_SERVICE,
	USIM_EST_NUM_OF_SERVICE
} USIM_EST_SERVICE_t;

/**
 Application types supported in USIM
 **/
typedef enum
{
	USIM_APPLICATION,
	ISIM_APPLICATION,
    WSIM_APPLICATION
} USIM_APPLICATION_TYPE;

/**
 RAT Mode Setting in AT&T EF-RAT for RAT balancing feature. The enum value is the same
 as defined in EF-RAT: do not change the enum value.
 **/
typedef enum
{
	USIM_RAT_DUAL_MODE = 0,
	USIM_RAT_GSM_ONLY = 1,
	USIM_RAT_UMTS_ONLY = 2,
	USIM_RAT_MODE_INVALID = 3
} USIM_RAT_MODE_t;

/// Result Data
typedef struct
{
	SIMAccess_t result;
} USIM_FILE_UPDATE_RSP_t;

/**
 * @addtogroup CAPI2_SIMAPIGroup
 * @{
 */

//***************************************************************************************
/**
    Function to return whether a USIM application is supported in the inserted USIM. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		appli_type Application type supported in USIM

	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_USIM_IS_APP_SUPPORTED_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : Boolean, TRUE if the passed application type is supported; FALSE otherwise.
**/	
void CAPI2_USIM_IsApplicationSupported(UInt32 tid, UInt8 clientID,USIM_APPLICATION_TYPE appli_type);


//***************************************************************************************
/**
    Function to check if the passed APN is allowed by APN Control List (ACL) feature in USIM. 
	This function should be called only if a USIM is inserted. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		apn_name (in) APN name whose labels are separated by '.', e.g. "isp.cingular". 
	@see		When matching the passed APN name with those in EF-ACL, the comparison is case-insensitive.
				For example, "isp.cingular", "Isp.Cingular" and "ISP.CINGULAR" are considered the same. 
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_USIM_IS_APN_ALLOWED_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : Boolean, TRUE if the passed APN is allowed; FALSE otherwise. 
**/	
void CAPI2_USIM_IsAllowedAPN(UInt32 tid, UInt8 clientID, char *apn_name);


//***************************************************************************************
/**
    Function to return the number of APN's in EF-ACL in USIM. This function should be called
	only if a USIM is inserted. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_USIM_GET_NUM_APN_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : UInt8 ,Number of APN's. 
**/	
void CAPI2_USIM_GetNumOfAPN(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
    Function to get an APN in EF-ACL. This function should be called only if a USIM is inserted. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		index (in) 0-based index. 
	
	@return		::RESULT_OK if the APN name is returned successfully; ::RESULT_ERROR otherwise. 
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_USIM_GET_APN_ENTRY_RSP
	@n@b Result_t :		::RESULT_OK if the APN name is returned successfully; ::RESULT_ERROR otherwise. 
	@n@b ResultData : APN_NAME_t , APN name whose labels are separated by '.', e.g. "isp.cingular".
**/	
void CAPI2_USIM_GetAPNEntry(UInt32 tid, UInt8 clientID,UInt8 index);


//***************************************************************************************
/**
    Function to check the activated/deactivated status for a service defined in EF-EST. 
	This function should be called only if a USIM is inserted. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		est_serv (in) Service type in EF-EST.  
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_USIM_IS_EST_SERV_ACTIVATED_RSP
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : Boolean, TRUE if service is activated; FALSE otherwise. 
**/	
void CAPI2_USIM_IsEstServActivated(UInt32 tid, UInt8 clientID,USIM_EST_SERVICE_t est_serv);


//***************************************************************************************
/**
    Function to send a request to USIM to activate/deactivate a service defined in EF-EST. 
	This function should be called only if a USIM is inserted. A MSG_SIM_SET_EST_SERV_RSP 
	message will be passed back to the callback function. 
	
	The PIN2 must have been correctly verified before calling this function. Otherwise 
	the SIMACCESS_NEED_CHV2 error is returned in the MSG_SIM_SET_EST_SERV_RSP message. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param 		est_serv (in) Service type in EF-EST.  
	@param 		serv_on (in) TRUE to activate service; FALSE to deactivate service. 
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_SET_EST_SERV_RSP
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : USIM_FILE_UPDATE_RSP_t
**/	
void CAPI2_USIM_SendSetEstServReq(UInt32 tid, UInt8 clientID, USIM_EST_SERVICE_t est_serv, Boolean serv_on);


//***************************************************************************************
/**
    Function to send a request to USIM to update an APN in EF-ACL. This function should be 
	called only if a USIM is inserted. A MSG_SIM_UPDATE_ONE_APN_RSP message will be passed 
	back to the callback function. 
	
	The PIN2 must have been correctly verified before calling this function. Otherwise 
	the SIMACCESS_NEED_CHV2 error is returned in the MSG_SIM_UPDATE_ONE_APN_RSP message. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param 		index (in) 0-based index; Pass ADD_APN_INDEX for adding a new APN to EF-ACL.   
	@param 		apn_name (in) APN name whose labels are separated by '.', e.g. "isp.cingular".
							  Pass NULL to delete an APN from EF-ACL. 
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_UPDATE_ONE_APN_RSP
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : USIM_FILE_UPDATE_RSP_t
**/	
void CAPI2_USIM_SendWriteAPNReq(UInt32 tid, UInt8 clientID, UInt8 index, char *apn_name);


//***************************************************************************************
/**
    Function to send a request to USIM to delete all APN's in EF-ACL. This function should be 
	called only if a USIM is inserted. A MSG_SIM_DELETE_ALL_APN_RSP message will be passed 
	back to the callback function. 
	
	The PIN2 must have been correctly verified before calling this function. Otherwise 
	the SIMACCESS_NEED_CHV2 error is returned in the MSG_SIM_DELETE_ALL_APN_RSP message. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
				sim_access_cb (in) Callback function. 
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_DELETE_ALL_APN_RSP
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : USIM_FILE_UPDATE_RSP_t
**/	
void CAPI2_USIM_SendDeleteAllAPNReq(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    Function to get the setting in AT&T EF-RAT that specifies the RAT Mode Setting. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_USIM_GET_RAT_MODE_RSP
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : USIM_RAT_MODE_t
**/	
void CAPI2_USIM_GetRatModeSetting(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		service Service provided by the SIM
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_USIM_GET_SERVICE_STATUS_RSP
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : SIMServiceStatus_t
**/	
void CAPI2_USIM_GetServiceStatus(UInt32 tid, UInt8 clientID, SIMService_t service);


//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		apn_name Buffer to store the returned APN name whose labels are
				separated by '.', e.g. "isp.cingular".
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : SIMServiceStatus_t
**/	
void CAPI2_SIM_IsAllowedAPN(UInt32 tid, UInt8 clientID, char* apn_name);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : Boolean
**/	
void CAPI2_SIM_IsBdnOperationRestricted(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		prefer_plmn_file Identifies the SIM/USIM file regarding PLMN
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : None
**/	
void CAPI2_SIM_SendPreferredPlmnUpdateInd(UInt32 tid, UInt8 clientID, SIM_PLMN_FILE_t prefer_plmn_file);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : None
**/	
void CAPI2_SIM_Deactive(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		oper_state (in) FDN state setting
	@param		chv2 (in) PIN2 if not verified yet
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : None
**/	
void CAPI2_SIM_SendSetBdnReq(UInt32 tid, UInt8 clientID, SIMBdnOperState_t oper_state,CHVString_t chv2);

//***************************************************************************************
/**
	This function request copy of raw USIM UST Data
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The SIM module result is as follows based on interim async response
	@n@b Responses 
	@n@b MsgType_t : MSG_USIM_UST_DATA_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : USIM_UST_DATA_RSP_t
**/	
void CAPI2_USIM_GetUst(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function allows the AP to notify the CP the updated SIMLOCK status, e.g.  
	if the user has unlocked the Newwork Lock using the correct password. 
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		simlock_state (in) Updated SIMLOCK status
	@return		None
	
	@note
	There is no reponse for this funtion call. 
**/	
void CAPI2_SIMLOCK_SetStatus(UInt32 tid, UInt8 clientID, SIMLOCK_STATE_t *simlock_state);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		power_on (in) True to power on card, False to power off.
    @param      mode (in) SIM Power on mode (Generic or Normal)
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : None
**/	
void CAPI2_SIM_PowerOnOffCard(UInt32 tid, UInt8 clientID, Boolean power_on, SIM_POWER_ON_MODE_t mode);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : None
**/	
void CAPI2_SIM_GetRawAtr(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
    @param      protocol (in) Protocol (T=0,T=1) to set
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : None
**/	
void CAPI2_SIM_Set_Protocol(UInt32 tid, UInt8 clientID, UInt8 protocol);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b UInt8 :		::Protocol (T=0,T=1) to get.
	@n@b ResultData : None
**/	
void CAPI2_SIM_Get_Protocol(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
    @param      apdu (in) APDU Command data
    @param      len (in) Length of APDU Command
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b UInt8 :		::Protocol (T=0,T=1) to get.
	@n@b ResultData : None
**/	
void CAPI2_SIM_SendGenericApduCmd(UInt32 tid, UInt8 clientID, UInt8 *apdu, UInt32 len);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b UInt8 :		::Protocol (T=0,T=1) to get.
	@n@b ResultData : None
**/	
void CAPI2_SIM_TerminateXferApdu(UInt32 tid, UInt8 clientID);

//******************************************************************************
//
// Function Name:	SIM_SendWriteSmsReq
//
// Description:		This function writes a SMS Message (SMS).
//
// Notes:	a MSG_SIM_SMS_WRITE_RSP message will be returned to the passed 
//			callback function (maybe to be sent to another task queue).
//
//******************************************************************************
//***************************************************************************************
/**
    Function	SIM_SendWriteSmsReq
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		rec_no (in) rec_no
	@param		tp_pid (in) tp_pid
	@param		p_sms_mesg (in) SIM SMS Msg
	
	@note
	The async response
	@n@b Responses  : 
	@n@b MsgType_t : MSG_SIM_SMS_WRITE_RSP
	@n@b ResultData : SIM_SMS_UPDATE_RESULT_t
**/	
void CAPI2_SIM_SendWriteSmsReq(UInt32 tid, UInt8 clientID, UInt16 rec_no, UInt8 tp_pid, SIMSMSMesg_t *p_sms_mesg);

/** @} */

#ifdef __cplusplus
}
#endif


#endif  // _CAPI2_SIMAPI_H_


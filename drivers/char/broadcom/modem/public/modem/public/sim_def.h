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
*   @file   sim_def.h
*
*   @brief  This file defines the data types used in SIM API functions.
*
****************************************************************************/

#ifndef _SIM_DEF_H_
#define _SIM_DEF_H_
   

/**
 Mode for READ RECORD and UPDATE RECORD SIM command
**/
#define EFILE_RECORD_NEXT_MODE		0x02	///< Next Mode, applicable for Cyclic file only
#define EFILE_RECORD_PREVIOUS_MODE	0x03	///< Previous Mode, applicable for Cyclic file only
#define EFILE_RECORD_ABSOLUTE_MODE	0x04	///< Absolute Mode, applicable for both Linear-Fixed and Cyclic file


#define MAX_BINARY_FILE_DATA_LEN_PER_CMD 255 ///< Maximum number of bytes to read/write data in Binary file because of APDU limit


#define MAX_BINARY_FILE_DATA_LEN (MAX_BINARY_FILE_DATA_LEN_PER_CMD * 6)	///< Maximum number of bytes for SIM Binary File access


#define ADD_APN_INDEX 0xFF	///< Index for adding a new APN name to the EF-ACL into the first empty entry


#define UNKNOWN_PLMN_INDEX 	0xFFFF 	///< Indicate all PLMN entries are requested


#define USIM_BASIC_CHANNEL_SOCKET_ID 0	///< Socket ID for basic channel reserved for USIMAP application


#define MAX_SIM_FILE_PATH_LEN	5	///< Maximum length for the path of a SIM/USIM file


#define MAX_UST_LEN	50	///< Maximum number of bytes for EF-UST

#define MAX_AD_LEN 4	///< Maximum number of bytes in EF-AD

#define SIM_HZ_TAGS_LEN	12			// Tag length for each Zone ID 

#define MAX_FCI_LEN	 0xFF
#define MAX_APDU_LEN 260

typedef struct
{
	UInt8 ustData[MAX_UST_LEN];
	UInt8 ustLen;
} UST_DATA_t;

typedef struct
{
	UInt8 adData[MAX_AD_LEN];	// EF-AD data
	UInt8 adLen;		
} AD_DATA_t;

typedef UInt8 ECC_t[ECC_DIGITS + 1]; ///< Emergency Call Code (null-term.)


/// Define whether SIM supports GSM only, RUIM only or dual-mode (both GSM and RUIM). This is applicable only for 2G SIM. 
typedef enum 
{ 
	SIM_TYPE_SUPPORT_GSM_ONLY, 
	SIM_TYPE_SUPPORT_RUIM_ONLY, 
	SIM_TYPE_SUPPORT_GSM_RUIM 
} SIMSupport_t; 


typedef enum
{
	HOMEZONE_CITYZONE_IND_OFF,		///< HomeZone/CityZone indication should be cleared
	HOMEZONE_IND_ON,				///< HomeZone indication should be displayed to user
	CITYZONE_IND_ON,				///< CityZone indication should be displayed to user
	HOMEZONE_CITYZONE_IND_INVALID	///< HomeZone/CityZone indication status not initialized
} HomezoneCityzoneStatus_t;


typedef struct
{
	HomezoneCityzoneStatus_t status;		///< HomeZone/CityZone indication status
	UInt8	tag_len;						///< Number of bytes in "tag_data"		
	UInt8	tag_data[SIM_HZ_TAGS_LEN];		///< HomeZone/CityZone tags in GSM Default Alphabet
} HomezoneIndData_t;



/// APDU File ID
typedef enum
{
	APDUFILEID_INVALID_FILE		= 0x0000,

	// Master File
	APDUFILEID_MF				= 0x3F00,

	APDUFILEID_USIM_ADF			= 0x7FFF,
	APDUFILEID_ISIM_ADF			= 0x7FFF,

	APDUFILEID_DF_TELECOM		= 0x7F10,
	APDUFILEID_DF_GSM			= 0x7F20,

	APDUFILEID_DF_RUIM			= 0x7F25,

	/* DF's defined by Cingular for ENS (Enhanced Network Selection) feature */
	APDUFILEID_DF_CINGULAR		= 0x7F66,	///< APDUFILEID_DF_ACTNET & APDUFILEID_EF_CINGULAR_TST are under this DF 
	APDUFILEID_DF_ACTNET		= 0x5F30,	///< APDUFILEID_EF_ACTHPLMN is under this DF 

	/* German O2 HomeZone DF/EF */
	APDUFILEID_DF_HOMEZONE			= 0x7F43,	///< HomeZone DF defined by German O2 network

	APDUFILEID_EF_HOMEZONE_PARAM	= 0x6F60,	///< HomeZone parameters for CB-HZI-UE algorithm
	APDUFILDID_EF_HOMEZONE_CACHE1	= 0x6F61,	///< HomeZone cache for zone ID 1 for CB-HZI-UE algorithm
	APDUFILDID_EF_HOMEZONE_CACHE2	= 0x6F62,	///< HomeZone cache for zone ID 2 for CB-HZI-UE algorithm
	APDUFILDID_EF_HOMEZONE_CACHE3	= 0x6F63,	///< HomeZone cache for zone ID 3 for CB-HZI-UE algorithm
	APDUFILDID_EF_HOMEZONE_CACHE4	= 0x6F64,	///< HomeZone cache for zone ID 4 for CB-HZI-UE algorithm
	
	APDUFILDID_EF_HOMEZONE_TAGS		 = 0x6F80,  ///< HomeZone text tags for LC-HZI-UE algorithm
	APDUFILDID_EF_HOMEZONE_SUB_CELL1 = 0x6F81,	///< HomeZone subscribed LAC/CI-1 for LC-HZI-UE algorithm
	APDUFILDID_EF_HOMEZONE_SUB_CELL2 = 0x6F82,	///< HomeZone subscribed LAC/CI-2 for LC-HZI-UE algorithm
	APDUFILDID_EF_HOMEZONE_SUB_CELL3 = 0x6F83,	///< HomeZone subscribed LAC/CI-3 for LC-HZI-UE algorithm
	APDUFILDID_EF_HOMEZONE_SUB_CELL4 = 0x6F84,	///< HomeZone subscribed LAC/CI-4 for LC-HZI-UE algorithm

	APDUFILDID_EF_HOMEZONE_SETTING	 = 0x6F87,	///< HomeZone UHZIUE setting

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
	 * Sepcification (CPHS): under DF GSM. They are not defined in GSM 11.11 or 3GPP 31.102
	 */
	APDUFILEID_EF_MWI_CPHS		= 0x6F11,	///< voice message waiting
	APDUFILEID_EF_CFL_CPHS		= 0x6F13,	///< call forward flags
	APDUFILEID_EF_ONS_CPHS		= 0x6F14,	///< operator name string
	APDUFILEID_EF_CSP_CPHS		= 0x6F15,	///< customer service profile
	APDUFILEID_EF_INFO_CPHS		= 0x6F16,	///< CPHS info
	APDUFILEID_EF_MBN_CPHS		= 0x6F17,	///< mailbox numbers
	APDUFILEID_EF_ONSS_CPHS		= 0x6F18,	///< operator name string in shortform
	APDUFILEID_EF_IN_CPHS		= 0x6F19,	///< information numbers
	/* CPHS EF definitions end */

	APDUFILEID_EF_KC			= 0x6F20,
	APDUFILEID_EF_PLMNSEL		= 0x6F30,

	APDUFILEID_EF_RUIM_UIMID	= 0x6F31,	///< EF UIMID on RUIM SIM
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
	APDUFILEID_EF_UST_3G		= 0x6F38, /**< EF-UST, exists only in USIM */
	APDUFILEID_EF_NIA			= 0x6F51,
	APDUFILEID_EF_KC_GPRS		= 0x6F52,
	APDUFILEID_EF_LOCI_GPRS		= 0x6F53,
	APDUFILEID_EF_SUME			= 0x6F54,
	APDUFILEID_EF_EST_3G		= 0x6F56, /**< EF-EST, exists only in USIM */
	APDUFILEID_EF_ACL_3G		= 0x6F57, /**< EF-ACL, exists only in USIM */
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


/// Identifies the SIM/USIM files regarding PLMN
typedef enum
{
	SIM_PLMN_SEL,		///< EF-PLMNsel applicable for 2G SIM only
	USIM_PLMN_WACT,		///< EF-PLMNwAct applicable for 3G USIM only
	USIM_OPLMN_WACT,	///< EF-OPLMNwAct applicable for 3G USIM only
	USIM_HPLMN_WACT		///< EF-HPLMNwAct applicable for 3G USIM only
} SIM_PLMN_FILE_t;


/// PLMN data for writing multiple entries. 
typedef struct
{	
	UInt16		mcc;						///< MCC which may include the 3rd digit of MNC, e.g. 13F0 for AT&T in Sunnyvale, CA. If empty, pass 0xFFFF
	UInt8		mnc;						///< MNC, e.g. 0x71 for AT&T in Sunnyvale, CA. If empty, pass 0xFF
	Boolean		gsm_act_selected;			///< TRUE if GSM Access Technology selected	(For 2G SIM, it is always FALSE)
	Boolean		gsm_compact_act_selected;	///< TRUE if GSM Compact Access Technology selected	(For 2G SIM, it is always FALSE)
	Boolean		utra_act_selected;			///< TRUE if UTRAN Access Technology selected (For 2G SIM, it is always FALSE)
} SIM_MUL_PLMN_ENTRY_t;


/// Service Flag Status
typedef enum
{
	SERVICE_FLAG_EXIST_CLEARED,	///< The corresponding bit exists in EF-CSP and is cleared 
	SERVICE_FLAG_EXIST_SET,		///< The corresponding bit exists in EF-CSP and is set 
	SERVICE_FLAG_NOT_EXIST		///< The corresponding bit does not exist in EF-CSP 
} SERVICE_FLAG_STATUS_t;	///< Service status in EF-CSP


/// Service Code Status defined in CPHS
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


/// CPHS Service Table entries 
typedef enum
{
	OPNAME_SHORTFORM,	///< CPHS EF-ONSS provision status in SIM
	MAILBOX_NUMBERS,	///< CPHS EF-MBX provision status in SIM
	SST,				///< CPHS EF-SST provision status in SIM
	CSP,				///< CPHS EF-CSP provision status in SIM
	INFORMATION_NUMBERS	///< CPHS EF-Info-Num provision status in SIM
} CPHS_SST_ENTRY_t;


#define SMSPARAM_SCA_SZ	12	///< Service Center Address data size in EF-SMSP


typedef UInt8 SIM_SCA_DATA_t[SMSPARAM_SCA_SZ];	///< Service Center Address data in EF-SMSP

#define SMSPARAM_SCA_DIGIT_SZ ((SMSPARAM_SCA_SZ - 2) << 1) /* Minus 2 to exclude length and type of number bytes */


#define ICC_DIGITS	20		///< Maximum ICC identification digits


typedef UInt8 ICCID_BCD_t[ICC_DIGITS / 2];		///< ICCID digits in BCD format
typedef UInt8 ICCID_ASCII_t[ICC_DIGITS + 1];	///< NULL terminated ICCID string in ASCII format


/// Call Forward Unconditional status flags in CPHS EF-6F11 
typedef struct
{
	Boolean call_forward_l1;	///< Boolean indicating call forward on line 1
	Boolean call_forward_l2;	///< Boolean indicating call forward on line 2
	Boolean call_forward_fax;	///< Boolean indicating call forward on fax line
	Boolean call_forward_data;	///< Boolean indicating call forward on data line
} SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t;


/// SIM Call Forward Unconditional result
typedef struct
{
	SIMAccess_t result;								///< SIM access result
	Boolean		  sim_exist;						///< TRUE if EF-CFI exists in SIM
	SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t	value;	///< Call forwarding unconditional status in SIM
} SIM_CALL_FORWARD_UNCONDITIONAL_STATUS_t;


typedef enum
{
  SIM_APPL_2G,		///< 2G SIM is inserted
  SIM_APPL_3G,		///< 3G USIM is inserted
  SIM_APPL_INVALID	///< No SIM is inserted
} SIM_APPL_TYPE_t;	///< SIM application type


/// Generic ADPU definitions
typedef enum
{
    SIM_POWER_ON_GENERIC_MODE,			///< For generic application, e.g. BT-SAP
    SIM_POWER_ON_NORMAL_MODE,			///< For normal application, e.g. phone application and Java application
    SIM_POWER_ON_INVALID_MODE = 0xff	///< Invalid power on mode
}SIM_POWER_ON_MODE_t;


/// SIMLOCK status for a SIMLOCK type
typedef enum
{
    SIM_SECURITY_OPEN,		    ///< SIMLOCK/PhoneLock disabled or password verified
	SIM_SECURITY_LOCKED,	    ///< SIMLOCK/PhoneLock enabled & pending on password verification    
	SIM_SECURITY_BLOCKED,	    ///< SIMLOCK/PhoneLock blocked & may be unblocked if semi-permanent
	SIM_SECURITY_VERIFIED,      ///< SIMLOCK/PhoneLock password has been verified
	SIM_SECURITY_NOT_INIT       ///< SIMLOCK status not initialized yet
} SIM_SECURITY_STATE_t;


/// SIMLOCK status for all SIMLOCK types
typedef struct
{
	SIM_SECURITY_STATE_t network_lock;			///< Network Lock status
	SIM_SECURITY_STATE_t network_subset_lock;	///< Network Subset Lock status
	SIM_SECURITY_STATE_t service_provider_lock;	///< Service Provider Lock status
	SIM_SECURITY_STATE_t corporate_lock;		///< Corporate Lock status
	SIM_SECURITY_STATE_t phone_lock;			///< Phone Lock status
} SIMLOCK_STATE_t;								///< SIMLOCK status structure


/// SIM Phase
typedef enum
{
	SIMPHASE_1,			///< Phase 1 SIM
	SIMPHASE_2,			///< Phase 2 SIM
	SIMPHASE_2PLUS		///< Phase 2+ SIM
} SIMPhase_t;			///< SIM phase indicated in EF-AD


/// SIM present status
typedef enum
{
	SIMPRESENT_REMOVED,		///< SIM Romoved
	SIMPRESENT_INSERTED,	///< SIM Inserted
	SIMPRESENT_NO_STATUS,	///< No Status Available yet
	SIMPRESENT_ERROR		///< Fatal error
} SIMPresent_t;				///< SIM inserted/removed status


#define CHV_MIN_LENGTH	4	///< Min. number of digits in CHV
#define PUK_MIN_LENGTH	8	///< Min. number of digits in PUK
#define CHV_MAX_LENGTH	8	///< Max. number of digits in CHV
#define PUK_MAX_LENGTH	8	///< Min. number of digits in PUK


typedef UInt8 CHVString_t[CHV_MAX_LENGTH + 1]; ///< NULL terminated CHV string
typedef UInt8 PUKString_t[PUK_MAX_LENGTH + 1]; ///< NULL terminated PUK string


typedef enum
{
	SIMOPERSTATE_NO_OPERATION,				///< FDN status not initalized yet 
	SIMOPERSTATE_RESTRICTED_OPERATION,		///< FDN feature enabled in SIM
	SIMOPERSTATE_UNRESTRICTED_OPERATION		///< FDN feature disabled in SIM
} SIMOperState_t;							///< FDN feature enabled/disabled status


/// SIM BDN Operation state
typedef enum
{
	SIMBDNSTATE_NO_OPERATION,				///< BDN status not initalized yet 
	SIMBDNSTATE_RESTRICTED_OPERATION,		///< BDN feature enabled in SIM
	SIMBDNSTATE_UNRESTRICTED_OPERATION		///< BDN feature disabled in SIM
} SIMBdnOperState_t;						///< BDN feature enabled/disabled status


/// CHV type
typedef enum
{
	CHV1,	///< SIM PIN1
	CHV2	///< SIM PIN2
} CHV_t;	///< Select SIM PIN1 or PIN2


/// SIM phonebook ID
typedef enum
{
	SIMPBK_ADN,			///< Abbreviated Dialing Number Phonebook
	SIMPBK_FDN,			///< Fixed Dialing Number Phonebook
	SIMPBK_SDN,			///< Service Dialing Number Phonebook
	SIMPBK_LND,			///< Last Number Dialed Phonebook
	SIMPBK_MSISDN,		///< MSISDN Dialing Number Phonebook
	SIMPBK_BDN			///< Barred Dialing Number Phonebook
} SIMPBK_ID_t;			///< SIM Phonebook ID


#ifndef _COMMON_DEFS_H_
typedef UInt32 CallMeterUnit_t;	///< General Call Meter Unit
#endif

#define CURRENCY_SIZE	3	///< Currency name string size 


typedef UInt8 CurrencyName_t[CURRENCY_SIZE + 1]; ///< NULL terminated currency name string


/// EPPU
typedef struct
{
	UInt16 mant;	///< EPPU mantissa
	Int16 exp;		///< EPPU exponent (signed)
} EPPU_t;			///< Price Per Unit information


typedef enum
{
	SIMSERVICESTATUS_NOT_ALLOCATED1	= 0,	///< Service Status is unknown
	SIMSERVICESTATUS_NOT_ACTIVATED	= 1,	///< Service Status is allocated, but not activated
	SIMSERVICESTATUS_NOT_ALLOCATED2	= 2,	///< Service Status is not allocated
	SIMSERVICESTATUS_ACTIVATED		= 3		///< Service Status is allocated and activated
} SIMServiceStatus_t;						///< SIM service status in EF-SST (2G SIM) or EF-UST (3G USIM)


/* Note: Do not change the order of the following enum. The order is associated with
 *       the coding in function SIM_GetServiceStatus() in sim_mi.c.
 */
/// Services provided by the SIM
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
	SIMSERVICE_VGCS_VGCSS,					///< Voice Group Call Service 
	SIMSERVICE_VBS_VBSS,					///< Voice Broadcast Service
	SIMSERVICE_EMLPP,						///< Enhanced Multi-Level Precedence and Pre-emption
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
	SIMSERVICE_IMG,							///< Image (EF-IMG)
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

	SIMSERVICE_LOCAL_PHONEBK,				///< Exists only for USIM: Local phonebook support
	SIMSERVICE_OUTGOING_CALL_INFO,			///< Exists only for USIM: Out-going call phonebook
	SIMSERVICE_INCOMING_CALL_INFO,			///< Exists only for USIM: Incoming call phonebook
	SIMSERVICE_CBMIR,						///< Exists only for USIM: Cell Broadcast Identifier Range
	SIMSERVICE_GSM_ACCESS,					///< Exists only for USIM: EF-CBMIR, CBMI range support
	SIMSERVICE_ENABLED_SERVICE_TABLE,		///< Exists only for USIM: GSM access support
	SIMSERVICE_APN_CONTROL_LIST,			///< Exists only for USIM: APN Control List (ACL)
	SIMSERVICE_GSM_SECURITY_CONTEXT,		///< Exists only for USIM: GSM security context
	SIMSERVICE_EXT5,						///< Exists only for USIM: Extension 5
	SIMSERVICE_GPRS_CALL_CONTROL,			///< Exists only for USIM: Call Control on GPRS

	SIMSERVICE_RFU							///< SIM service undefined

} SIMService_t;								/// SIM service status in EF-SST (2G SIM) or EF-UST (3G USIM)


/// Common command values defined in Section 10.1.2 of 3GPP 31.101 & Section 9.2 of GSM 11.11 
typedef enum
{
	APDUCMD_SELECT				= 0xA4,	///< Select command
	APDUCMD_STATUS				= 0xF2, ///< Status Command
	
	APDUCMD_READ_BINARY			= 0xB0, ///< Read Binary command
	APDUCMD_UPDATE_BINARY		= 0xD6, ///< Update Binary command
	APDUCMD_READ_RECORD			= 0xB2, ///< Record record command
	APDUCMD_UPDATE_RECORD		= 0xDC, ///< Update record command
	
	APDUCMD_SEEK				= 0xA2,	///< Called "Seek" command in GSM 11.11, but called "Search Record" in 3GPP 31.101 
	APDUCMD_INCREASE			= 0x32, ///< Increase command

	APDUCMD_VERIFY_CHV			= 0x20,	///< Verify CHV command
	APDUCMD_CHANGE_CHV			= 0x24,	///< Change CHV command
	APDUCMD_DISABLE_CHV			= 0x26,	///< Disable CHV command
	APDUCMD_ENABLE_CHV			= 0x28, ///< Enable CHV command
	APDUCMD_UNBLOCK_CHV			= 0x2C, ///< Unblock CHV command

	APDUCMD_INVALIDATE			= 0x04,	///< Called "Invalidate" in GSM 11.11, but called "Deactivate File" in 3GPP 31.101  
	APDUCMD_REHABILITATE		= 0x44, ///< Called "Rehabilitate" in GSM 11.11, but called "Activate File" in 3GPP 31.101  
	APDUCMD_AUTHENTICATE		= 0x88, ///< Called "Run GSM Algorithm" in GSM 11.11, but called "Authenticate" in 3GPP 31.101  

	APDUCMD_SLEEP				= 0xFA,	///< This is old GSM command not supported, it is not valid in USIM 

	APDUCMD_TERMINAL_PROFILE	= 0x10, ///< Terminal Profile command
	APDUCMD_ENVELOPE			= 0xC2, ///< Envelope command
	APDUCMD_FETCH				= 0x12, ///< Fetch command
	APDUCMD_TERMINAL_RESPONSE	= 0x14, ///< Terminal Response command
	APDUCMD_MANAGE_CHANNEL		= 0x70,	///< Manage Channel command

	APDUCMD_GET_RESPONSE		= 0xC0	///< Get Response command
} APDUCmd_t;							///< All command values for commands sent to SIM


/// SIM PIN status
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

	SIM_ABSENT_STATUS,				///< SIM is not inserted
	SIM_ERROR_STATUS,				///< SIM operation error: should never get this status if everything works	
	SIM_PUK_BLOCK_STATUS,			///< SIM is permanently blocked because SIM PUK attempt exceeded, normal UE function not possible
	SIM_PUK2_BLOCK_STATUS,			///< SIM PIN2 is permanently blocked because PUK2 attempt exceeded, but normal UE function is OK
} SIM_PIN_Status_t;


#define IMSI_DIGITS	15	///< Max. IMSI digits


typedef UInt8 IMSI_t[IMSI_DIGITS + 1];	///< NULL terminated IMSI string in ASCII format


#define GID_DIGITS	10	///< Max. GID1/GID2 file length


typedef UInt8 GID_DIGIT_t[GID_DIGITS];	///< GID1/GID2 file length (not null terminated)


/// Application types supported in USIM
typedef enum
{
	USIM_APPLICATION,	///< USIM application
	ISIM_APPLICATION,	///< ISIM (IMS SIM) application
	PKCS15_APPLICATION,	///< PKCS-15 application defined in http://www.rsa.com/rsalabs/node.asp?id=2141
    WSIM_APPLICATION	///< WSIM (WAP SIM) application
} USIM_APPLICATION_TYPE;


#define MAX_APN_NAME_LEN 40	///< Maximum length for APN name


typedef char APN_NAME_t[MAX_APN_NAME_LEN + 1];	///< NULL terminated APN name string


/** 
 Activated/Deactivated status for services defined in EF-EST. The following enum's must be defined 
 in the same order as that in Section 4.2.47 of 31.102.
**/ 
typedef enum
{
	USIM_EST_FDN_SERVICE,		///< FDN feature	
	USIM_EST_BDN_SERVICE,		///< BDN feature
	USIM_EST_ACL_SERVICE,		///< ACL (APN control list) feature
	USIM_EST_NUM_OF_SERVICE		///< Number of services in EF-EST
} USIM_EST_SERVICE_t;


/** 
 RAT Mode Setting in AT&T EF-RAT for RAT balancing feature. The enum value is the 
 same as defined in EF-RAT: do not change the enum value
**/
typedef enum
{
	USIM_RAT_DUAL_MODE 		= 0,	///< UE is dual mode
	USIM_RAT_GSM_ONLY 		= 1,	///< UE is GSM only
	USIM_RAT_UMTS_ONLY 		= 2,	///< UE is UMTS only (AT&T spec says they will not enable this mode)
	USIM_RAT_MODE_INVALID 	= 3		///< RAT mode not initialized yet
} USIM_RAT_MODE_t;					


/// Response parameters after selecting a MF/DF. See Section 9.2.1 of GSM 11.11 
typedef struct
{
	SIMAccess_t result;				///< SIM access result
	UInt16 mem_not_allocated;		///< Total memory not allocated under directory
	UInt8 file_charact;				///< File characteristics
	UInt8 num_df;					///< Number of DF's which are direct children of directory
	UInt8 num_ef;					///< Number of EF's which are direct children of directory
	UInt8 num_chv;					///< Number of unlock CHV codes, unblock CHV codes and adminstrative codes
	
	Boolean chv1_code_initialized;	///< TRUE if CHV1 code is initialized
	UInt8 chv1_unlock_attempt;		///< Number of remaining attempts to unlock CHV1
	UInt8 chv1_unblock_attempt;		///< Number of remaining attempts to unblock CHV1

	Boolean chv2_code_initialized;	///< TRUE if CHV2 code is initialized
	UInt8 chv2_unlock_attempt;		///< Number of remaining attempts to unlock CHV2
	UInt8 chv2_unblock_attempt;		///< Number of remaining attempts to unblock CHV2
	UInt8 sw1;						///< SW1 returned by card in case of an error. 
	UInt8 sw2;						///< SW2 returned by card in case of an error. 
	UInt8 fciLen;					///< valid data in fci
	UInt8 fci[MAX_FCI_LEN];			///< undecoded response to select APDU
} SIM_DFILE_INFO_t;


/// EF file types
typedef enum
{
	TRANSPARENT_EFILE,		///< Transparent file
	LINEAR_FIXED_EFILE,		///< Linear fixed file
	CYCLIC_EFILE,			///< Cyclic file
	INVALID_EFILE			///< Invalid SIM file type
} EFILE_TYPE_t;


/// Access condition defined in Section 9.3 of GSM 11.11 
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


/// Life Cycle Status of a 3G USIM EF: see Section 11.1.1.4.9 of 31.101 
typedef enum
{
	NO_INFO_GIVEN_STATE,
	CREATION_STATE,
	INITIALIZATION_STATE,
	OPERATIONAL_ACTIVATED_STATE,
	OPERATIONAL_DEACTIVATED_STATE,
	TERMINATION_STATE
} USIM_LIFE_CYCLE_STATYS_t;



/// SIM/USIM EFile Configuration Information
typedef struct
{
	SIMAccess_t result;			///< SIM access result
	UInt16 file_size;			///< File size					
	UInt8 record_length;		///< record length (applicable only for linear fixed or cyclic record)
	EFILE_TYPE_t efile_type;	///< Type: Transparent; Linear Fixed; Cyclic

	/* Access condition defined in Section 9.3 of GSM 11.11: ONLY applicable for 2G SIM */
	EFILE_ACCESS_CONDITION_t update_access_cond;		///< Update access condition for 2G SIM
	EFILE_ACCESS_CONDITION_t read_seek_access_cond;		///< Read access condition for 2G SIM
	EFILE_ACCESS_CONDITION_t increase_access_cond;		///< Increase access condition for 2G SIM
	EFILE_ACCESS_CONDITION_t invalidate_access_cond;	///< Invalidate access condition for 2G SIM
	EFILE_ACCESS_CONDITION_t rehabilitate_access_cond;	///< Rehabilitate access condition for 2G SIM

	/* Access condition defined in Section 7.1 of ISO/IEC 7816-4 and Section 9.5.1 of 3GPP 31.101. 
	 * These access conditions are for 3G USIM only, not applicable for 2G SIM.
	 */
	USIM_EFILE_ACCESS_CONDITION_t	usim_update_access_cond;	///< Update accesss condition for 3G USIM, including Update Binary & Update Record commands
	USIM_EFILE_ACCESS_CONDITION_t	usim_read_access_cond;		///< read access condition for 3G USIM: ncluding Read Binary, Read Record, Search Binary, Search Record commands
	USIM_EFILE_ACCESS_CONDITION_t	usim_activate_access_cond;	///< Activate access condition for 3G USIM
	USIM_EFILE_ACCESS_CONDITION_t	usim_deactivate_access_cond;	///< Deactivate access condition for 3G USIM

	/* Invalidation status: ONLY applicable for 2G SIM */
	Boolean validated;				///< TRUE if file is validated
	Boolean accessible_invalidated;	///< Readable & updatable when invalidated
	
	USIM_LIFE_CYCLE_STATYS_t life_cycle_satus;	///< Life cycle status: ONLY applicable for 3G SIM
	UInt8		sw1;		///< SW1 returned by card in case of an error 
	UInt8		sw2;		///< SW2 returned by card in case of an error
	UInt8 fciLen;			///< valid data in fci
	UInt8 fci[MAX_FCI_LEN];	///< undecoded response to select APDU
} SIM_EFILE_INFO_t;


/// SIM EF Data
typedef struct
{
	SIMAccess_t result;		///< SIM access result
	UInt8		sw1;		///< SW1 returned by card in case of an error 
	UInt8		sw2;		///< SW2 returned by card in case of an error
	UInt16		data_len;	///< Length of data returned.
	UInt8		*ptr;		///< Pointer to data buffer (no need for client to free buffer because it justs points to intertask message body)
} SIM_EFILE_DATA_t;


/// EFile Update Result
typedef struct
{
	SIMAccess_t result;		///< SIM access result
	UInt8		sw1;				///< SW1 returned by card in case of an error. 
	UInt8		sw2;				///< SW2 returned by card in case of an error.
} SIM_EFILE_UPDATE_RESULT_t;


/// SIM Search Record (Seek Command in 2G SIM) Response Data
typedef struct
{
	SIMAccess_t result;		///< SIM access result
	UInt8		data_len;	///< Length of data returned.
	UInt8		*ptr;		///< Pointer to data buffer which stores 1-based record indices (no need for client to free 
							///< buffer because it justs points to intertask message body)
} SIM_SEEK_RECORD_DATA_t;


/// Number of PIN/PUK attempts remaining
typedef struct
{
	SIMAccess_t result;		 ///< SIM access result
    UInt8 pin1_attempt_left; ///< Number of PIN1 attempts remaining
    UInt8 pin2_attempt_left; ///< Number of PIN2 attempts remaining
    UInt8 puk1_attempt_left; ///< Number of PUK1 attempts remaining
    UInt8 puk2_attempt_left; ///< Number of PUK2 attempts remaining
} PIN_ATTEMPT_RESULT_t;


/// Number of PLMN entry response in a preferred PLMN file
typedef struct
{
	SIMAccess_t result;				///< SIM access result
	SIM_PLMN_FILE_t plmn_file;		///< PLMN file
	UInt16 num_of_total_entry;		///< Number of total entries, including empty entries
} SIM_PLMN_NUM_OF_ENTRY_t;


/// PLMN entry data in preferred PLMN file
typedef struct
{
	UInt16		index;						///< 0-based index of the PLMN entry
	UInt16		mcc;						///< MCC which may include the 3rd digit of MNC, e.g. 13F0 for AT&T in Sunnyvale, CA. Pass 0xFFFF if empty
	UInt8		mnc;						///< MNC, e.g. 0x71 for AT&T in Sunnyvale, CA. Pass 0xFF if empty
	Boolean		gsm_act_selected;			///< TRUE if GSM Access Technology selected	(For 2G SIM, it is always FALSE)
	Boolean		gsm_compact_act_selected;	///< TRUE if GSM Compact Access Technology selected	(For 2G SIM, it is always FALSE)
	Boolean		utra_act_selected;			///< TRUE if UTRAN Access Technology selected (For 2G SIM, it is always FALSE)
} SIM_PLMN_ENTRY_t;


#define MAX_NUM_OF_PLMN 150 	///< Maximum of PLMN entries in SIM file


/// Read PLMN entry response
typedef struct
{
	SIMAccess_t result;								///< SIM access result
	SIM_PLMN_FILE_t plmn_file;						///< PLMN file
	UInt8 num_of_entry;								///< Number of entries in "plmn_entry", including empty entries
	SIM_PLMN_ENTRY_t plmn_entry[MAX_NUM_OF_PLMN];	///< PLMN entry data
} SIM_PLMN_ENTRY_DATA_t;


/// Update multiple PLMN entries response
typedef struct
{
	SIMAccess_t result;				///< SIM access result
	SIM_PLMN_FILE_t plmn_file;		///< PLMN file
	UInt16 start_index;				///< 0-based index to which the start PLMN is written
	UInt16 num_of_entry;            ///< Number of PLMN entries that are written   
} SIM_MUL_PLMN_ENTRY_UPDATE_t;


/// Generic APDU command result
typedef enum
{
    SIM_GENERIC_APDU_RES_SUCCESS,
    SIM_GENERIC_APDU_RES_FAILURE,
    SIM_GENERIC_APDU_RES_NOT_ACCESSABLE,
    SIM_GENERIC_APDU_RES_POWERED_OFF,
    SIM_GENERIC_APDU_RES_REMOVED,
    SIM_GENERIC_APDU_RES_POWERED_ON,
    SIM_GENERIC_APDU_RES_NOT_AVAIL
} SIM_GENERIC_APDU_RESULT_t;


/// Generic APDU command event
typedef enum
{
   SIM_GENERIC_APDU_POWER_ON_OFF_EVT = 1,	///< Power on/off SIM response
   SIM_GENERIC_APDU_CARD_STATUS = 2,		///< SIM card status
   SIM_GENERIC_APDU_ATR_RESP = 3,			///< SIM ATR response
   SIM_GENERIC_APDU_XFER_RESP = 4			///< Transfer APDU response
} SIM_GENERIC_APDU_EVENT_t;


/// Generic APDU command event and result
typedef struct
{						
    SIM_GENERIC_APDU_EVENT_t eventType;		///< APDU event type
    SIM_GENERIC_APDU_RESULT_t resultCode;	///< APDU result status
}SIM_GENERIC_APDU_RES_INFO_t;


#define SIM_GENERIC_APDU_MAX_ATR_LEN 34		///< Maximum ATR data response length
#define SIM_GENERIC_APDU_MAX_CMD_LEN 255	///< Maximum APDU command length
#define SIM_GENERIC_APDU_MAX_RSP_LEN 255	///< Maximum APDU response response


/// ATR response data
typedef struct
{
    SIM_GENERIC_APDU_EVENT_t eventType;			///< APDU request event type
    SIM_GENERIC_APDU_RESULT_t resultCode;		///< APDU result code
    UInt8 len;									///< ATR data length
    UInt8 data[SIM_GENERIC_APDU_MAX_ATR_LEN];	///< ATR data
}SIM_GENERIC_APDU_ATR_INFO_t;


/// SIM response structure for Generic APDU command
typedef struct
{
    SIM_GENERIC_APDU_RESULT_t resultCode;		///< APDU result code
    UInt16 len;									///< APDU response data length
    UInt8 data[SIM_GENERIC_APDU_MAX_RSP_LEN];	///< APDU response data
}SIM_GENERIC_APDU_XFER_RSP_t;


/// SIM Phonebook Configuration Information
typedef struct
{
	SIMAccess_t result;		///< SIM access result
	SIMPBK_ID_t pbk;		///< Phonebook type
	UInt16 total;			///< Total number of entries in phone book
	UInt16 alpha_sz;		///< Size of alpha part of entry
} SIM_PBK_INFO_t;


/// SIM Maximum Accumulated Call Meter structure
typedef struct
{
	SIMAccess_t result;			///< SIM access result
	CallMeterUnit_t max_acm;	///< Max value of the ACM
} SIM_MAX_ACM_t;


/// SIM Accumulated Call Meter value
typedef struct
{
	SIMAccess_t result;		///< SIM access result
	CallMeterUnit_t acm;	///< Value of the ACM
} SIM_ACM_VALUE_t;


#define SVC_PROV_SIZE	16	///< Service Provider string size


typedef UInt8 SvcProvName_t[SVC_PROV_SIZE + 1]; ///< NULL terminated Service provider name
										

/// SIM Service Provider name
typedef struct
{
	SIMAccess_t result;		///< SIM access result
	Boolean display_flag1;	///< 2G SIM:TRUE, registered PLMN display is mandatory,
							///< 3G USIM: TRUE,mandatory if registered PLMN is HPLMN or in Service Provider List                              
    Boolean display_flag2;	///< 2G SIM: N/A, 3G USIM:TRUE, Service Provide display is not required
							///< if registered PLMN is HPLMN or in Service Provider List    
	SvcProvName_t name;		///< Service Provider Name string
} SIM_SVC_PROV_NAME_t;


/// SIM Price Per Unit data
typedef struct
{
	SIMAccess_t result;			///< SIM access result
	CurrencyName_t currency;	///< 3-char currency code string which is NULL terminated
	EPPU_t eppu;				///< Elementary Price Per Unit
} SIM_PUCT_DATA_t;


/// Restricted SIM access response data
typedef struct
{
	SIMAccess_t result;	///< SIM access result
	UInt8 *data;		///< The data returned from SIM if available (no need for client to free the buffer because the poiner points to intertask message body)
	UInt16 data_len;	///< data length
	UInt8 sw1;			///< status word SW1 byte
	UInt8 sw2;			///< status word SW2 byte
} SIM_RESTRICTED_ACCESS_DATA_t;


/// SIM EF update result 
typedef struct
{
	SIMAccess_t result;		///< EF update result
} USIM_FILE_UPDATE_RSP_t;


/// SIM Service flag status
typedef struct
{
	SIMAccess_t result;				///< SIM access result
	SERVICE_FLAG_STATUS_t value;	///< Service flag status
} SIM_SERVICE_FLAG_STATUS_t;


/// Read SIM ICCID result
typedef struct
{
	SIMAccess_t result;		///< SIM access result
	ICCID_ASCII_t iccid;	///< NULL terminated ICCID string
} SIM_ICCID_STATUS_t;


typedef UInt8  SIM_GENERIC_APDU_CMD_BUF_t[SIM_GENERIC_APDU_MAX_CMD_LEN];	///< APDU command buffer


/// Response for Open Logical Channel request
typedef struct
{
	UInt8 socket_id;		///< 0 if failure in opening logical channel; non-0 channe ID value otherwise
} SIM_OPEN_SOCKET_RES_t;


/// Status for activating non-USIM Application request
typedef struct
{
	SIMAccess_t result;		///< SIMACCESS_SUCCESS if non-USIM application is successfully activated
} SIM_SELECT_APPLI_RES_t;


/// Status for activating non-USIM Application request
typedef struct
{
	SIMAccess_t result;		///< SIMACCESS_SUCCESS if non-USIM application is successfully activated
} SIM_SELECT_APPLICATION_RES_t;


/// Status for deactivating non-USIM Application request
typedef struct
{
	SIMAccess_t result;		///< SIMACCESS_SUCCESS if non-USIM application is successfully deactivated
} SIM_DEACTIVATE_APPLI_RES_t;


/// Status for Close Logical Channel request
typedef struct
{
	SIMAccess_t result;		///< SIMACCESS_SUCCESS if logical channel is successfully closed
} SIM_CLOSE_SOCKET_RES_t;


// Maximum number of bytes to read for a multiple record read operation (limited by 65K CAPI2 buffer size)
#define MAX_MUL_REC_DATA_SIZE 60000

/// Multiple record data
typedef struct
{
	SIMAccess_t result;		///< SIM access result
	UInt8	num_of_rec;		///< Number of records returned
	UInt8	rec_len;		///< Length of each record
	UInt8	*rec_data;		///< Pointer to the data buffer, no need to deallocate since it is part of the intertask msg data payload
} SIM_MUL_REC_DATA_t;


#define MAX_CPHS_ONS_LEN	30	///< Maximum size of CPHS ONS string
#define MAX_CPHS_ONSS_LEN	10	///< Maximum size of CPHS ONSS string


typedef UInt8 CPHS_ONS_t[MAX_CPHS_ONS_LEN];		///< CPHS_ONS data
typedef UInt8 CPHS_ONSS_t[MAX_CPHS_ONSS_LEN];	///< CPHS_ONSS data


#define	SST_SIZE	15		///< Maximum size for EF-SST data in 2G SIM


typedef UInt8 SST_t[SST_SIZE];	///< SIM Service Table in 2G SIM


#define CPHS_INFO_SIZE 3	///< EF-CPHS-Info data size


typedef UInt8 CPHS_INFO_t[CPHS_INFO_SIZE];	///< EF-CPHS-Info data in SIM


/// SIM data needed to check SIMLOCK status
typedef struct
{
	IMSI_t imsi_string;	///< NULL terminated IMSI string
	GID_DIGIT_t	gid1;	///< GID1 data
	UInt8 gid1_len;		///< Number of bytes in "gid1" element, i.e. number of bytes in EF-GID1
	GID_DIGIT_t gid2;	///< GID2 data
	UInt8 gid2_len;		///< Number of bytes in "gid2" element, i.e. number of bytes in EF-GID2
} SIMLOCK_SIM_DATA_t;


#define MAX_NUM_OF_SIM_LANGUAGE	15 ///< Maximum number of languages in SIM files: EF-ELP & EF-LP in 2G SIM; EF-LI & EF-PL in 3G USIM.


/// Preferred Language Setting in 2G SIM. Section 11.2.1 of GSM 11.11 defines language preference in 2G SIM. 
typedef struct
{
	/** EF-ELP language list in ISO-639 encoding: each language is represented by a
	pair of alpha-numberic characters, e.g. "EN" (0x454E) for English. 0xFFFF or 0x0000 if not defined 
	**/
	UInt16	elp_language_list[MAX_NUM_OF_SIM_LANGUAGE];	
	
	/** EF-LP language list in CB lauguage encoding: defined in Section 5 of GSM 03.38. 
	Each byte defines a language, e.g. 0x01 for English. 0xFF if not defined 
	**/
	UInt8	lp_language_list[MAX_NUM_OF_SIM_LANGUAGE];	
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
	SIM_APPL_TYPE_t		sim_appl_type;	///< Inserted SIM type: SIM_APPL_INVALID if SIM not inserted; otherwise SIM_APPL_2G or SIM_APPL_3G
	SIM_LANGUAGE_INFO_t	lang_info;		///< SIM language list: applicable only if SIM inserted (sim_appl_type is SIM_APPL_2G or SIM_APPL_3G)
	Boolean				ruim_supported;	///< TRUE if the inserted SIM is RUIM (CDMA2000 SIM)
	UInt8               sim_slot;		///< 1 if associated with phyisical SIM slot 1; 2 if associated with physical SIM slot 2
} SIM_DETECTION_t;

/// APDU data structure
typedef struct
{
	UInt32		length;			///< length of valid part of data buffer
	UInt8		data[261];		///< buffer for APDU data 
} APDU_t;


typedef struct
{
	SIMAccess_t	result;		///< SIM access result
	APDU_t		rApdu;		///< APDU returned by card
} SimApduRsp_t;

/// MSG_SIM_PIN_IND
typedef struct
{
	//need to make this appl. specific
	Boolean Pin1Enabled;
	Boolean Pin1Verified;
	Boolean Pin1Blocked;
	Boolean Pin1PukBlocked;
	Boolean Pin2Enabled;
	Boolean Pin2Verified;
	Boolean Pin2Blocked;
	Boolean Pin2PukBlocked;
} SimPinInd_t;

typedef struct
{
    Boolean     swValid;    ///< indicates if the following status words were returned by the card
                            ///< status words are only valid if the APDU was actually sent to the card and an error happened
    UInt8       sw1;        ///< SW1 card response to a VERIFY/CHANGE/DISABLE/ENABLE/UNBLOCK PIN APDU in case of an error
    UInt8       sw2;        ///< SW2 card response to a VERIFY/CHANGE/DISABLE/ENABLE/UNBLOCK PIN APDU in case of an error
} PinStatusWord_t;

typedef struct
{
	SIMAccess_t	    result; ///< SIM access result
	PinStatusWord_t sw;
} SimPinRsp_t;

/// SIM fatal error condition
typedef enum
{
	SIM_IMSI_NOT_EXIST,		///< Mandatory EF-IMSI does not exist or is empty. MMI shall indicate invalid SIM to user, but can choose to continue to access other SIM files, such as phonebook
	SIM_CONTINUAL_TECH_RPOBLEM,	///< SIM continally sends SW1/SW2=0x6F/0x00 tech problem status, i.e. AT&T Phonebook feature
	SIM_EST_UST_NOT_EXIST,  ///< Mandatory EF-SST (if 2G SIM) or EF-UST (if 3G USIM) does not exist 
	SIM_ARR_NOT_EXIST,      ///< EF-ARR does not exist & there is a file whose security attribute uses "8B" tag. See section 13.4 of 3GPP 31.102. This error is sent for 3G USIM only. 
	SIM_SELECT_FILE_ERROR,  ///< Select File error. DF in file info does not match DF in cache
	SIM_FATAL_ERROR_UNKNOWN ///< Unknown fatal error 
} SIM_FATAL_ERROR_t;


/// USIM AID data that can be passed to SimApi_SendSelectAppiReq() function to activate an application.
typedef struct
{
	UInt8 aidData[80];
	UInt8 aidLen;
} USIM_AID_DATA_t;


// SIM Lock Callback function

typedef void (SIMLockCbackFunc_t)(SIMLOCK_STATE_t *simlock_state, const SIMLOCK_SIM_DATA_t *sim_data, Boolean is_test_sim);

typedef enum
{
    SIM_INTERFACE_INVALID,
	SIM_INTERFACE_ISO,
	SIM_INTERFACE_ICCD
} SIM_SIM_INTERFACE_t;

typedef enum
{
    /**
	This SIM message is to notify ATC/MMI notify that the SIM card has been recovered. 
	**/
	MSG_CAPI_SIM_SIM_RECOVER_IND	            = 0x0C84,	///<Payload type: none
} SimMsgType_t;

typedef enum
{
    SIM_APP_OCCUR_FIRST,
    SIM_APP_OCCUR_LAST,
    SIM_APP_OCCUR_NEXT,
    SIM_APP_OCCUR_PREV
} SIM_APP_OCCURRENCE_t;

#endif  // _SIM_DEF_H_

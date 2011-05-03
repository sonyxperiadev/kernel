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


/**
 * @addtogroup CAPI2_STKAPIGroup
 * @{
 */

// defined in sysparm.h #define MAX_TERMINAL_PROFILE_ARRAY_SIZE		30

#define MAX_SATK_ENVELOPE_CMD_LEN 261
#define MAX_SATK_TERMINAL_RSP_LEN 261
#define MAX_STK_BROWSING_STATUS_LEN 32
#define STK_SS_DATA_LEN          255
#define STK_SMS_ADDRESS_LEN      254



/**
Term Profile
**/
typedef struct {
	UInt8 length;					///< Length of Terminal Profile data returned. 
	UInt8* data;					///< Pointer to the Terminal Profile data.
} CAPI2_TermProfile_t; 

/** STK Call control setup response.
** Data payload for normal Call Control (non-SS and non_USSD) response message
**/
typedef struct
{
	StkCallControlResult_t cc_result;	///< Call Control return result from SIM 
	StkCallControl_t old_type;	///< Call type when we sent the Call Control request 
	StkCallControl_t new_type;	///< Call type specified by SIM Call Control 
	
	Boolean addr_changed;		///< TRUE if the called party address is new or changed 
	Boolean bc1_changed;		///< TRUE if the BC1 parameters are new or changed 
	Boolean subaddr_changed;	///< TRUE if the called party subaddress is new or changed 
	Boolean alpha_id_changed;	///< TRUE if alpha identifier is new or changed 
	Boolean bc_repeat_changed;	///< TRUE if BC repeat indicator is new or changed 
	Boolean bc2_changed;		///< TRUE if the BC2 parameters are new or changed 
	
	/* Called party address parameters */
	gsm_TON_t ton;	///< Called party type of number
	gsm_NPI_t npi;	///< Called party nuber plan identifier

	char number[MAX_DIGITS + 1];	///< ASCII encoded dialling number 

	/* BC1 data */
	BearerCapability_t bc1; ///< Bearer capability 1 data

	/* Called Party Subaddress data */
	Subaddress_t subaddr_data; ///< Called part sub address

	/* Alpha Identifier data */
	Boolean alpha_id_valid;		///< TRUE if alpha identifier data is valid */
	UInt8 alpha_id_len;			///< Alpha ID length
	UInt8 alpha_id[ALPHA_ID_LEN];///< Alpha ID data

	BearerCapability_t bc2; ///< Bearer capability 2 data
	UInt8 bc_repeat;	///< BC Repeat Indicator

} StkCallControlSetupRsp_t;

/**
Data payload for SS Call Control response message 
**/
typedef struct
{
	StkCallControlResult_t cc_result;	///< Call Control return result from SIM 
	StkCallControl_t old_type;	///< Call type when we sent the Call Control request 
	StkCallControl_t new_type;	///< Call type specified by SIM Call Control 

	Boolean ss_str_changed;		///< TRUE if the SS dialing string is new or changed 

	UInt8	ton_npi;	///< TON/NPI byte: one of UNKNOWN_TON_UNKNOWN_NPI (128), UNKNOWN_TON_ISDN_NPI (129) & INTERNA_TON_ISDN_NPI (145)
	UInt8	ss_len;				///< Length of SS dialing string 
	UInt8	ss_data[MAX_DIGITS / 2]; ///< SS dialing string 

	/* Alpha Identifier data */
	Boolean alpha_id_valid;		///< TRUE if alpha identifier data is valid 
	UInt8 alpha_id_len;		///< Alpha ID length
	UInt8 alpha_id[ALPHA_ID_LEN];///< Alpha ID data

} StkCallControlSsRsp_t;

/**
Data payload for USSD Call Control response message 
**/
typedef struct
{
	StkCallControlResult_t cc_result;	///< Call Control return result from SIM 
	StkCallControl_t old_type;	///< Call type when we sent the Call Control request 
	StkCallControl_t new_type;	///< Call type specified by SIM Call Control 

	Boolean ussd_str_changed;		///< TRUE if the USSD dialing string is new or changed 

	UInt8	ussd_len;				///< Length of USSD dialing string 
	UInt8	ussd_data[PHASE2_MAX_USSD_STRING_SIZE+1]; ///< USSD dialing string 

	/* Alpha Identifier data */
	Boolean alpha_id_valid;		///< TRUE if alpha identifier data is valid 
	UInt8 alpha_id_len;			///< Alpha ID length
	UInt8 alpha_id[ALPHA_ID_LEN];///< Alpha ID data

	UInt8 ussd_data_dcs;			/// dcs for USSD string
} StkCallControlUssdRsp_t;

/**
Data payload for MO SMS Call Control response message 
**/
typedef struct
{
	StkCallControlResult_t cc_result;	///< Call Control return result from SIM */

	Boolean sca_changed;		///< TRUE if the service centre number is changed */
	Boolean dest_changed;		///< TRUE if destination number is changed */

	/* Service Center Number data */
	UInt8 sca_toa;				///< Service Center Address type of address
	UInt8 sca_number_len;		///< SCA number length
	UInt8 sca_number[SMS_MAX_DIGITS / 2]; ///<SCA number

	/* Destination number data */
	UInt8 dest_toa;		///< Destination type of address
	UInt8 dest_number_len;	///< Destination number length
	UInt8 dest_number[SMS_MAX_DIGITS / 2];///< Destination number

	/* Alpha Identifier data */
	Boolean alpha_id_valid;		///< TRUE if alpha identifier data is valid 
	UInt8 alpha_id_len;			///< Alpha ID length
	UInt8 alpha_id[ALPHA_ID_LEN];///< Alpha ID data

} StkCallControlSmsRsp_t;

/** @} */

#endif
